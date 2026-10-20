#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit "add left, right".
static inline uint32_t addFlags32(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// Fixed-point reciprocal: eax = (0x10000 + divisor/2) / divisor, rounded to nearest, with a zero
// divisor treated as one. ecx and edx are preserved. The flags are those of "add eax, edx"
// (div leaves them alone in the oracle).
// FUNCTION: GP4 0x004126bc
void __cdecl FUN_004126bc(gp4::Regs* r) {
    uint32_t divisor = r->eax;
    if (divisor == 0u) {
        divisor = 1u;
    }
    const uint32_t half = divisor >> 1;
    const uint32_t dividend = 0x10000u + half;

    r->eax = dividend / divisor;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(0x10000u, half, dividend);
}
GP4_IMPL(0x004126bc, FUN_004126bc, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
