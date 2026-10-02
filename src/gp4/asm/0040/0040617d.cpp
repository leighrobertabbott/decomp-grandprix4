#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0040617d
void __cdecl FUN_0040617d(gp4::Regs* r) {
    const uint32_t left = static_cast<uint16_t>(r->eax);
    const uint32_t right = GP4_GLOBAL(uint32_t, 0x00866f08);
    const uint32_t value = left + right;
    const uint32_t flags = (value < left ? 1u : 0u)
        | parityFlag(value)
        | ((left ^ right ^ value) & 0x10u)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ value) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040617d, FUN_0040617d, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
