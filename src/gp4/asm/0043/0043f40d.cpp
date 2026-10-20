#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Next segment record along the lap / pit lane: +0x160, wrapped by the lap
// length below the pit start, clamped at the pit end.
static inline uint32_t nextSegment(uint32_t seg) {
    seg += 0x160u;
    if (seg >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
        if (seg < GP4_GLOBAL(uint32_t, 0x00891500)) {
            seg -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        } else if (seg >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            seg = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    }
    return seg;
}

// Previous segment record, wrapped forward under the lap start, clamped at the
// pit start.
static inline uint32_t prevSegment(uint32_t seg) {
    if (seg >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        seg -= 0x160u;
        if (seg < GP4_GLOBAL(uint32_t, 0x00891500)) {
            seg = GP4_GLOBAL(uint32_t, 0x00891500);
        }
    } else {
        seg -= 0x160u;
        if (seg < GP4_GLOBAL(uint32_t, 0x00891504)) {
            seg += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    }
    return seg;
}

// FUN_0043fab0 on a segment record, sign-selected by the direction byte.
static inline int32_t signedGap(const gp4::Regs* r, uint32_t seg) {
    gp4::Regs call = *r;
    call.edi = seg;
    gp4::call_regs(0x0043fab0, call);
    int32_t gap = static_cast<int32_t>(call.eax);
    if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0) {
        gap = -gap;
    }
    return gap;
}

static inline int32_t signedWord16(uint32_t seg) {
    int32_t value = GP4_FIELD(int16_t, seg, 0x16);
    if (GP4_GLOBAL(uint8_t, 0x00627b65) == 0) {
        value = -value;
    }
    return value;
}

static inline int32_t absWord16(uint32_t seg) {
    int32_t value = GP4_FIELD(int16_t, seg, 0x16);
    if (value < 0) {
        value = -value;
    }
    return value;
}

// FUNCTION: GP4 0x0043f40d
void __cdecl FUN_0043f40d(gp4::Regs* r) {
    uint32_t cur = r->edi;
    uint32_t mark = 0;
    uint32_t limit = 0;
    int32_t steps = 0;
    int32_t prevValue = 0;
    int32_t value = 0;

    GP4_GLOBAL(uint8_t, 0x00627b65) = 0;
    GP4_GLOBAL(uint32_t, 0x00627ac8) = cur;
    if ((GP4_FIELD(uint8_t, cur, 0x73) & 2) != 0) {
        GP4_GLOBAL(uint8_t, 0x00627b65) = 0xff;
    }

    // Loop 1: find the end of the walk (final or 0x20 record, or leaving the lap).
    for (;;) {
        if ((GP4_FIELD(uint8_t, cur, 0x73) & 0x10) != 0) {
            goto finish;
        }
        if ((GP4_FIELD(uint8_t, cur, 0x73) & 0x20) != 0) {
            break;
        }
        cur = nextSegment(cur);
        steps++;
        if (!(cur < GP4_GLOBAL(uint32_t, 0x00866ecc))) {
            break;
        }
    }

    // Loop 2: after 10 or more steps, retrace the walk from the start.
    if (steps >= 10) {
        if (signedGap(r, cur) < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00627ac0))) {
            limit = cur;
            cur = GP4_GLOBAL(uint32_t, 0x00627ac8);
            if (cur != limit) {
                steps = 0;
                do {
                    if (steps >= 6) {
                        if (signedGap(r, cur) < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00627abc))) {
                            goto finish;
                        }
                    }
                    cur = nextSegment(cur);
                    steps++;
                } while (cur != limit);
            }
        }
    }

    // Loop 3: walk until a final record or until the value stops rising.
    mark = cur;
    steps = 0;
    prevValue = signedWord16(cur);
    for (;;) {
        steps++;
        cur = nextSegment(cur);
        value = signedWord16(cur);
        if ((GP4_FIELD(uint8_t, cur, 0x73) & 0x10) != 0) {
            if (steps == 0) {
                goto finish;
            }
            uint32_t back = (static_cast<uint32_t>(steps) >> 1) * 0x160u;
            if (cur >= GP4_GLOBAL(uint32_t, 0x00891500)) {
                cur -= back;
                if (cur < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    cur = GP4_GLOBAL(uint32_t, 0x00891500);
                }
            } else {
                cur -= back;
                if (cur < GP4_GLOBAL(uint32_t, 0x00891504)) {
                    cur += GP4_GLOBAL(uint32_t, 0x00866ec4);
                }
            }
            uint32_t before = prevSegment(cur);
            if (absWord16(before) < absWord16(cur)) {
                cur = before;
            }
            goto finish;
        }
        if (value - static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00627b04)) < prevValue) {
            cur = mark;
            goto finish;
        }
        prevValue = value;
        mark = cur;
    }

finish:
    GP4_FIELD(uint8_t, cur, 0x73) |= 0x80;
    if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0) {
        GP4_FIELD(uint8_t, cur, 0x73) |= 2;
    } else {
        GP4_FIELD(uint8_t, cur, 0x73) &= 0xfd;
    }
    const uint32_t result = GP4_FIELD(uint8_t, cur, 0x73);
    // or/and on a byte: CF = OF = AF = 0, PF/ZF/SF from the stored byte.
    r->eflags = (r->eflags & ~0x8d5u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 7) << 7);
}
GP4_IMPL(0x0043f40d, FUN_0043f40d, "regs(edi:rec[0x8943a0,0x160,0..23]) -> (cf, pf, af, zf, sf, of); globals=layout=ring2_0043f40d; cases=2000")
