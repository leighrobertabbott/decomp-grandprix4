#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Two word-table lookups at index eax, each biased by -0x8031: edx = first table entry, eax = second.
// The flags are those of the final 16-bit "sub ax, 0x8031".
// FUNCTION: GP4 0x0043cd01
void __cdecl FUN_0043cd01(gp4::Regs* r) {
    const uint32_t index = r->eax;
    const uint32_t first = GP4_ARRAY(uint16_t, 0x0062eaf4)[index];
    const uint32_t second = GP4_ARRAY(uint16_t, 0x0062eb0a)[index];
    const uint32_t result = (second - 0x8031u) & 0xffffu;

    const uint32_t flags = (second < 0x8031u ? 0x1u : 0u)
        | parityFlag(result)
        | (((second ^ 0x8031u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 8) & 0x80u)
        | ((((second ^ 0x8031u) & (second ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);

    r->edx = (first - 0x8031u) & 0xffffu;
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043cd01, FUN_0043cd01, "regs(eax:u32[0..10]) -> (eax, edx, cf, pf, af, zf, sf, of)")
