#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0040350c
void __cdecl FUN_0040350c(gp4::Regs* r) {
    const uint32_t position = r->edi;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    const uint32_t value = r->eax;
    const bool below = position < limit;
    const bool isOne = value == 1u;

    // The final flags are those of "cmp eax, 1" with CF forced by clc/stc:
    // below: stc unless eax == 1; at/above: stc only when eax == 1.
    const uint32_t result = value - 1u;
    const uint32_t carry = (below ? !isOne : isOne) ? 1u : 0u;
    const uint32_t flags = carry
        | parityFlag(result)
        | (((value ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((value ^ 1u) & (value ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040350c, FUN_0040350c, "regs(eax:i32[0..2], edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
