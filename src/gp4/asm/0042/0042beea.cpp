#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `test` (CF = OF = AF = 0).
static inline uint32_t logicFlags16(uint16_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

static inline uint16_t abs16(uint16_t value) {
    return static_cast<int16_t>(value) < 0 ? static_cast<uint16_t>(0u - value) : value;
}

static inline uint8_t* bytesAt(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x0042beea
// Car pair esi / ebx (pushal ... popal: every register is restored). The gap word [0x603e70] is set to
// the +0xe2 difference of the two cars (or 0x0040dc8a when either has bit 1 of +0x9c) for the duration
// and restored at the end. The record step [0x603ea0] starts at 0x160 (0 when word +0x9a of ebx is below
// 0x3ab) and 0x0042b17e evaluates the pair; CF = 1 (stc) when it finds nothing. Otherwise [0x604404] is
// copied to [0x604406] and, when the pass count [0x603eac] = min(((+0x9a + +0x9a) / 2 * 3 >> 13) + 1, 6)
// - 1 is nonzero and the cars or the segment byte +0x51 (bits 0x20, 0x80, 8 against the sign of
// [0x604404]) allow it, the step is raised by 0x2c0 per pass and 0x0042b17e rerun until it fails, the
// gap word changes sign or shrinks by less than the word [0x603eb4]; such a pass is undone (the four
// result words [0x604404], [0x604402], [0x603d9c], [0x6077c7] get their previous values back). CF = 0
// (clc) then; the other flags are those of the last test / cmp / shr / dec.
void __cdecl FUN_0042beea(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.st_xin_mask = 0;
    s.st_xout_mask = 0;

    const uint32_t savedGap = GP4_GLOBAL(uint32_t, 0x00603e70);

    uint8_t* self = bytesAt(s.esi);
    uint8_t* other = bytesAt(s.ebx);
    uint16_t gap;
    if ((GP4_FIELD(uint8_t, self, 0x9c) & 2u) == 0u && (GP4_FIELD(uint8_t, other, 0x9c) & 2u) == 0u) {
        gap = static_cast<uint16_t>(GP4_FIELD(uint16_t, other, 0xe2) - GP4_FIELD(uint16_t, self, 0xe2));
    } else {
        gp4::call_regs(0x0040dc8a, s);
        gap = static_cast<uint16_t>(s.eax);
    }
    GP4_GLOBAL(uint16_t, 0x00603e70) = gap;

    s.edi = GP4_FIELD(uint32_t, self, 0x1c);
    GP4_GLOBAL(uint8_t, 0x006077c9) = 0;

    const uint32_t speedOther = GP4_FIELD(uint16_t, other, 0x9a);
    const uint32_t speedSelf = GP4_FIELD(uint16_t, self, 0x9a);
    uint32_t passes = ((((speedOther + speedSelf) >> 1) * 3u) >> 13) + 1u;
    if (static_cast<int32_t>(passes) > 6) {
        passes = 6;
    }
    GP4_GLOBAL(uint32_t, 0x00603ea0) = 0x160u;
    if (GP4_FIELD(int16_t, other, 0x9a) < 0x3ab) {
        GP4_GLOBAL(uint32_t, 0x00603ea0) = 0u;
    }
    GP4_GLOBAL(uint32_t, 0x00603eac) = passes - 1u;

    s.edx = speedSelf;
    s.eax = GP4_GLOBAL(uint32_t, 0x00603ea0);
    gp4::call_regs(0x0042b17e, s);
    if ((s.eflags & 1u) != 0u) {
        GP4_GLOBAL(uint32_t, 0x00603e70) = savedGap;
        r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d4u) | 1u;
        return;
    }

    GP4_GLOBAL(uint16_t, 0x00604406) = GP4_GLOBAL(uint16_t, 0x00604404);
    const uint32_t remaining = GP4_GLOBAL(uint32_t, 0x00603eac);
    uint32_t flags;
    bool search = false;
    if (remaining == 0u) {
        flags = 0x44u;  // cmp eax, 0: equal
    } else {
        self = bytesAt(s.esi);
        other = bytesAt(s.ebx);
        if ((GP4_FIELD(uint8_t, self, 0x9c) & 2u) != 0u || (GP4_FIELD(uint8_t, other, 0x9c) & 2u) != 0u
            || (GP4_FIELD(uint8_t, other, 0x203) & 0x10u) != 0u) {
            search = true;
        } else {
            const uint8_t segmentFlags = GP4_FIELD(uint8_t, bytesAt(s.edi), 0x51);
            if ((segmentFlags & 0x20u) == 0u) {
                flags = 0x44u;  // test byte, 0x20: zero result
            } else {
                // Search while the sign of [0x604404] is the one the bits 0x80 / 8 ask for.
                const bool wantNegative = (((segmentFlags >> 7) ^ (segmentFlags >> 3)) & 1u) != 0u;
                const uint16_t current = GP4_GLOBAL(uint16_t, 0x00604404);
                if ((static_cast<int16_t>(current) < 0) == wantNegative) {
                    search = true;
                } else {
                    flags = logicFlags16(current);
                }
            }
        }
    }

    if (search) {
        GP4_GLOBAL(uint32_t, 0x00603eb0) = remaining;
        for (;;) {
            GP4_GLOBAL(uint32_t, 0x00603ea0) += 0x2c0u;
            const uint16_t keepGap = GP4_GLOBAL(uint16_t, 0x00604404);
            const uint16_t keepB = GP4_GLOBAL(uint16_t, 0x00604402);
            const uint16_t keepHigh = GP4_GLOBAL(uint16_t, 0x006077c7);
            const uint16_t keepLow = GP4_GLOBAL(uint16_t, 0x00603d9c);
            s.ecx = (s.ecx & 0xffff0000u) | keepGap;
            s.ebp = (s.ebp & 0xffff0000u) | keepB;
            s.edx = (static_cast<uint32_t>(keepHigh) << 16) | keepLow;
            const uint32_t savedEcx = s.ecx;
            const uint32_t savedEdx = s.edx;
            const uint32_t savedEbp = s.ebp;

            s.eax = GP4_GLOBAL(uint32_t, 0x00603ea0);
            gp4::call_regs(0x0042b17e, s);
            s.edx = savedEdx;
            s.ecx = savedEcx;
            s.ebp = savedEbp;

            bool undo = (s.eflags & 1u) != 0u;
            if (!undo) {
                const uint16_t now = GP4_GLOBAL(uint16_t, 0x00604404);
                if (static_cast<int16_t>(static_cast<uint16_t>(now ^ keepGap)) < 0) {
                    undo = true;
                } else {
                    const uint16_t shrink = static_cast<uint16_t>(abs16(keepGap) - abs16(now));
                    if (static_cast<int16_t>(shrink) < GP4_GLOBAL(int16_t, 0x00603eb4)) {
                        undo = true;
                    }
                }
            }
            if (undo) {
                GP4_GLOBAL(uint16_t, 0x00604404) = keepGap;
                GP4_GLOBAL(uint16_t, 0x00604402) = keepB;
                GP4_GLOBAL(uint16_t, 0x00603d9c) = keepLow;
                GP4_GLOBAL(uint16_t, 0x006077c7) = keepHigh;
                // shr edx, 0x10: the result is keepHigh (SF = OF = AF = 0)
                flags = parityFlag(keepHigh) | (keepHigh == 0u ? 0x40u : 0u);
                break;
            }
            const uint32_t left = GP4_GLOBAL(uint32_t, 0x00603eb0) - 1u;
            GP4_GLOBAL(uint32_t, 0x00603eb0) = left;
            if (left == 0u) {
                flags = 0x44u;  // dec to zero
                break;
            }
        }
    }

    GP4_GLOBAL(uint32_t, 0x00603e70) = savedGap;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u);
}
GP4_IMPL(0x0042beea, FUN_0042beea, "regs(eax:u32, ebx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677, ecx:u32, edx:u32, esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd")
