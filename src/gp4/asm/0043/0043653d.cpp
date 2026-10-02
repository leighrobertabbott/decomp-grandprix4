#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0043653d
void __cdecl FUN_0043653d(gp4::Regs* r) {
    const uint32_t left = r->esi;
    const uint32_t right = GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint32_t value = left - right;
    const uint32_t flags = (left < right ? 1u : 0u)
        | parityFlag(value)
        | ((left ^ right ^ value) & 0x10u)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ value) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043653d, FUN_0043653d, "regs(esi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
