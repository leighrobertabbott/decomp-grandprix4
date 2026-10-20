#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00414f09
// Clamps eax to a three-way step: >= 0x2000 -> 0x4000, >= -0x2000 -> 0, else -0x4000.
// The flags left behind are those of the last compare executed (the movs keep them).
void __cdecl FUN_00414f09(gp4::Regs* r) {
    const int32_t value = static_cast<int32_t>(r->eax);
    uint32_t result;
    uint32_t flags;
    if (value >= 0x2000) {
        result = 0x4000u;
        flags = cmpFlags32(static_cast<uint32_t>(value), 0x2000u);
    } else if (value >= -0x2000) {
        result = 0u;
        flags = cmpFlags32(static_cast<uint32_t>(value), 0xffffe000u);
    } else {
        result = 0xffffc000u;
        flags = cmpFlags32(static_cast<uint32_t>(value), 0xffffe000u);
    }
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00414f09, FUN_00414f09, "regs(eax:i32[-0x4000..0x4000]) -> (eax, cf, pf, af, zf, sf, of)")
