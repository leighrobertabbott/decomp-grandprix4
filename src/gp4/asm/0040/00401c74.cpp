#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00401c74
// With x = eax (Q14 angle-like argument), s = 0x401bc2(x) (the Q14 series, 1.0 = 0x4000):
//   |x| >= 0x1000: eax = ((s - 0x4000) << 14) / x, the flags are those of "shl eax, 14" (the idiv
//                   keeps them in the oracle);
//   |x| <  0x1000: eax = (((((x >> 2) + 0x4000) * x / 0xc000 + 0x4000) * x >> 14) >> 1) + 0x4000, the
//                   first stages of the 0x401bc2 polynomial; the flags are those of the final add.
// ebp, ecx and edx are saved and restored.
void __cdecl FUN_00401c74(gp4::Regs* r) {
    const int32_t x = static_cast<int32_t>(r->eax);
    const uint32_t magnitude = x < 0 ? 0u - static_cast<uint32_t>(x) : static_cast<uint32_t>(x);

    if (magnitude >= 0x1000u) {
        gp4::Regs series = *r;
        series.st_in = 0;
        series.st_out = 0;
        gp4::call_regs(0x00401bc2, series);

        const int32_t difference = static_cast<int32_t>(series.eax - 0x4000u);
        int32_t remainder;
        const int32_t quotient = gp4::idiv64(static_cast<int64_t>(difference) << 14, x, &remainder);

        // shl eax, 14 (pinned oracle model): CF = last bit out, AF = 0, OF = bit31(shift by 13 ^ shift by 14)
        const uint32_t value = static_cast<uint32_t>(difference);
        const uint32_t shifted = value << 14;
        const uint32_t flags = ((value >> 18) & 1u)
            | parityFlag(shifted)
            | (shifted == 0u ? 0x40u : 0u)
            | ((shifted & 0x80000000u) != 0u ? 0x80u : 0u)
            | ((((value << 13) ^ shifted) & 0x80000000u) != 0u ? 0x800u : 0u);

        r->eax = static_cast<uint32_t>(quotient);
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    const int64_t first = static_cast<int64_t>((x >> 2) + 0x4000) * static_cast<int64_t>(x);
    int32_t remainder;
    int32_t accumulator = gp4::idiv64(first, 0xc000, &remainder) + 0x4000;

    const int64_t second = static_cast<int64_t>(accumulator) * static_cast<int64_t>(x);
    accumulator = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14)) >> 1;

    const uint32_t before = static_cast<uint32_t>(accumulator);
    r->eax = before + 0x4000u;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(before, 0x4000u);
}
GP4_IMPL(0x00401c74, FUN_00401c74, "regs(eax:i32[-0x6000..0x6000]) -> (eax, cf, pf, af, zf, sf, of)")
