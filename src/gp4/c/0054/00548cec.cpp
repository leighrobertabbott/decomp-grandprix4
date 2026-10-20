#include <gp4/gp4.h>
#include <gp4/flags.h>

// Quotient of edx:eax by the divisor when it fits (divisor above the high dword), else the divisor itself.
static inline uint32_t Quotient_00548cec(uint64_t product, uint32_t divisor) {
    if (divisor > static_cast<uint32_t>(product >> 32)) {
        uint32_t rem;
        return gp4::div64(product, divisor, &rem);
    }
    return divisor;
}

// FUNCTION: GP4 0x00548cec
// pushal/popal routine. [0x831a5d] = 0xcd1 when word [0x7ad8ac] is set, else 0x14a0 (3281 / 5280: feet per
// km / mile). [0x831a59] = 10 * word +4 of the 12-byte entry [0x7ad894] of 0x7c3ae4, [0x831a55] its 32-bit
// product with the smaller of words [0x7c2e9c] / [0x7c2e96]. For car (byte [0x7c31dc] & 0x3f) - 1 (stored to
// [0x831a4d]) the dword 0x7c3208[car] goes to [0x831a3d], and [0x831a45] = [0x831a55] * 360000000 / it (the
// dword itself when the quotient would not fit) / [0x831a5d]. The same with [0x831a59], the order byte
// [0x675958] ([0x831a51]) and 0x675970[] gives [0x831a41] and [0x831a49]. Every GPR is restored; the flags
// are those of the last `cmp [0x831a5d], 0` (div leaves them in the oracle).
void __cdecl FUN_00548cec(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00831a5d) = (GP4_GLOBAL(uint16_t, 0x007ad8ac) != 0u) ? 0xcd1u : 0x14a0u;

    uint8_t* entry = GP4_ARRAY(uint8_t, 0x007c3ae4) + GP4_GLOBAL(uint32_t, 0x007ad894) * 0xcu;
    GP4_GLOBAL(uint32_t, 0x00831a59) = static_cast<uint32_t>(GP4_FIELD(uint16_t, entry, 4)) * 10u;
    uint32_t laps = GP4_GLOBAL(uint16_t, 0x007c2e9c);
    if (laps > GP4_GLOBAL(uint16_t, 0x007c2e96)) {
        laps = GP4_GLOBAL(uint16_t, 0x007c2e96);
    }
    GP4_GLOBAL(uint32_t, 0x00831a55) = GP4_GLOBAL(uint32_t, 0x00831a59) * laps;

    const uint32_t carIndex = (GP4_GLOBAL(uint8_t, 0x007c31dc) & 0x3fu) - 1u;
    GP4_GLOBAL(uint32_t, 0x00831a4d) = carIndex;
    GP4_GLOBAL(uint32_t, 0x00831a3d) = GP4_ARRAY(uint32_t, 0x007c3208)[carIndex];
    uint32_t value = Quotient_00548cec(static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x00831a55)) * 360000000u,
                                       GP4_GLOBAL(uint32_t, 0x00831a3d));
    if (GP4_GLOBAL(uint32_t, 0x00831a5d) > 0u) {
        value = value / GP4_GLOBAL(uint32_t, 0x00831a5d);
    } else {
        value = GP4_GLOBAL(uint32_t, 0x00831a5d);
    }
    GP4_GLOBAL(uint32_t, 0x00831a45) = value;

    const uint32_t orderIndex = (GP4_GLOBAL(uint8_t, 0x00675958) & 0x3fu) - 1u;
    GP4_GLOBAL(uint32_t, 0x00831a51) = orderIndex;
    GP4_GLOBAL(uint32_t, 0x00831a41) = GP4_ARRAY(uint32_t, 0x00675970)[orderIndex];
    value = Quotient_00548cec(static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x00831a59)) * 360000000u,
                              GP4_GLOBAL(uint32_t, 0x00831a41));
    const uint32_t divisor = GP4_GLOBAL(uint32_t, 0x00831a5d);
    if (divisor > 0u) {
        value = value / divisor;
    } else {
        value = divisor;
    }
    GP4_GLOBAL(uint32_t, 0x00831a49) = value;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(divisor, 0u));
}
GP4_IMPL(0x00548cec, FUN_00548cec, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+order_00675958+time_0040ed93+circ_007c3ae8")
