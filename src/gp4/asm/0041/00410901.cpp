#include <gp4/gp4.h>

static inline uint32_t comparisonFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00410901
void __cdecl FUN_00410901(gp4::Regs* r) {
    const uint32_t input = r->eax & 0xffu;
    const uint32_t index = r->edx < 17u ? r->edx : 0u;
    const uint32_t factor = GP4_ARRAY(uint8_t, 0x0062eade)[index];
    const uint32_t quotient = (input * factor + 50u) / 100u;
    r->eax = quotient < 3u ? 3u : quotient;
    r->eflags = (r->eflags & ~0x8d5u) | comparisonFlags32(quotient, 3u);
}
GP4_IMPL(0x00410901, FUN_00410901, "regs(eax:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=scale_0062eade")
