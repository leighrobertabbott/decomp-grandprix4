#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00425be8
void __cdecl FUN_00425be8(gp4::Regs* r) {
    const uint32_t left = r->eax;
    const uint32_t right = r->edx;
    const uint32_t sum = left + right;
    const uint32_t result = (sum >> 1) | (sum & 0x80000000u);
    const uint32_t flags = (sum & 1u) | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00425be8, FUN_00425be8, "regs(eax:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of)")
