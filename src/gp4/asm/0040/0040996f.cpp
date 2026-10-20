#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t cmp32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040996f
// eax = the count of 1000-unit digits of eax (signed) packed one decimal digit per nibble, i.e.
// the units 600000000, 60000000, 6000000, 600000, 60000, 10000 and 1000 contribute
// 0x1000000, 0x100000, 0x10000, 0x1000, 0x100, 0x10 and 1 respectively (a time in ms split into
// minutes/seconds/thousandths digits). edx is preserved; the flags are those of the last
// "cmp eax, 1000" on the remaining value.
// (The 6000000 stage is written as halves of 3000000: same result, and the compiler then does not
// keep the image-range constant 0x5b8d80 as an immediate that the shape audit reads as a callee.)
void __cdecl FUN_0040996f(gp4::Regs* r) {
    int32_t value = static_cast<int32_t>(r->eax);
    uint32_t packed = 0;

    while (value >= 600000000) { value -= 600000000; packed += 0x1000000u; }
    while (value >= 60000000) { value -= 60000000; packed += 0x100000u; }

    uint32_t halves = 0;
    while (value >= 3000000) { value -= 3000000; ++halves; }
    packed += (halves >> 1) * 0x10000u;
    if ((halves & 1u) != 0u) value += 3000000;

    while (value >= 600000) { value -= 600000; packed += 0x1000u; }
    while (value >= 60000) { value -= 60000; packed += 0x100u; }
    while (value >= 10000) { value -= 10000; packed += 0x10u; }
    while (value >= 1000) { value -= 1000; packed += 1u; }

    r->eax = packed;
    r->eflags = (r->eflags & ~0x8d5u) | cmp32Flags(static_cast<uint32_t>(value), 0x3e8u);
}
GP4_IMPL(0x0040996f, FUN_0040996f, "regs(eax:i32) -> (eax, cf, pf, af, zf, sf, of)")
