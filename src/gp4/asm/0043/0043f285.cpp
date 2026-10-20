#include <gp4/gp4.h>

// A 0x160-byte track-segment record (array at 0x8943a0); only the fields used here are named.
struct Unknown_0043f285_Segment {
    uint8_t Unknown00[0x16];
    int16_t Unknown16;
    uint8_t Unknown18[0x73 - 0x18];
    uint8_t Flags73;
};

static inline Unknown_0043f285_Segment* segmentAt(uint32_t seg) {
    return reinterpret_cast<Unknown_0043f285_Segment*>(static_cast<uintptr_t>(seg));
}

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
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
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0043fab0, call);
    int32_t gap = static_cast<int32_t>(call.eax);
    if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0) {
        gap = -gap;
    }
    return gap;
}

static inline int32_t signedWord16(uint32_t seg) {
    int32_t value = segmentAt(seg)->Unknown16;
    if (GP4_GLOBAL(uint8_t, 0x00627b65) == 0) {
        value = -value;
    }
    return value;
}

// FUNCTION: GP4 0x0043f285
// Backward counterpart of 0x0043f40d: walks the segment ring from the record in edi against the
// driving direction (prevSegment) and marks the record where the walk ends with byte +0x73 bit 6
// (0x40) and bit 1 = the direction byte 0x627b65 (set from the start record's bit 1). Loop 1 stops at
// a record with bit 0x80 (mark it) or 0x10; after 10 or more steps loop 2 retraces from the start to
// look for the first record (>= 6 steps in) whose gap falls under [0x627ab8]; otherwise loop 3 walks
// back until the signed word +0x16 minus [0x627b04] drops. pushal/popal: every register is
// preserved; the flags are those of the final or/and on the marked record's byte.
void __cdecl FUN_0043f285(gp4::Regs* r) {
    uint32_t cur = r->edi;
    int32_t steps = 0;
    uint32_t limit = 0;
    uint32_t mark = 0;
    int32_t prevValue = 0;
    int32_t value = 0;

    GP4_GLOBAL(uint8_t, 0x00627b65) = 0;
    GP4_GLOBAL(uint32_t, 0x00627ac8) = cur;
    if ((segmentAt(cur)->Flags73 & 2) != 0) {
        GP4_GLOBAL(uint8_t, 0x00627b65) = 0xff;
    }

    // Loop 1: walk back to a final (0x10) record or one marked 0x80.
    for (;;) {
        if ((segmentAt(cur)->Flags73 & 0x80) != 0) {
            goto finish;
        }
        if ((segmentAt(cur)->Flags73 & 0x10) != 0) {
            break;
        }
        cur = prevSegment(cur);
        steps++;
    }

    // Loop 2: after 10 or more steps, retrace the walk from the start.
    if (steps >= 10) {
        if (signedGap(r, cur) < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00627ab4))) {
            limit = cur;
            cur = GP4_GLOBAL(uint32_t, 0x00627ac8);
            if (cur != limit) {
                steps = 0;
                do {
                    if (steps >= 6) {
                        if (signedGap(r, cur) < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00627ab8))) {
                            goto finish;
                        }
                    }
                    cur = prevSegment(cur);
                    steps++;
                } while (cur != limit);
            }
        }
    }

    // Loop 3: keep walking back while the value does not drop.
    mark = cur;
    prevValue = signedWord16(cur);
    for (;;) {
        cur = prevSegment(cur);
        value = signedWord16(cur);
        if ((segmentAt(cur)->Flags73 & 0x80) != 0) {
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
    segmentAt(cur)->Flags73 |= 0x40;
    if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0) {
        segmentAt(cur)->Flags73 |= 2;
    } else {
        segmentAt(cur)->Flags73 &= 0xfd;
    }
    const uint32_t result = segmentAt(cur)->Flags73;
    // or/and on a byte: CF = OF = AF = 0, PF/ZF/SF from the stored byte.
    r->eflags = (r->eflags & ~0x8d5u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 7) << 7);
}
GP4_IMPL(0x0043f285, FUN_0043f285, "regs(edi:rec[0x8943a0,0x160,0..29]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_0043f285")
