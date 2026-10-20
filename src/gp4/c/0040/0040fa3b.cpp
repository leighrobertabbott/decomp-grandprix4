#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? 0x4u : 0u;
}

// FUNCTION: GP4 0x0040fa3b
void __cdecl FUN_0040fa3b(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint8_t, 0x0095404e);
    if (value != 0xffu) {
        // cmp byte [0x95404e], 0xff ; jne -> return with the flags of the cmp.
        const uint32_t diff = (value - 0xffu) & 0xffu;
        const uint32_t flags = (value < 0xffu ? 0x1u : 0u)
            | parityFlag(diff)
            | (((value ^ 0xffu ^ diff) & 0x10u) != 0u ? 0x10u : 0u)
            | (diff == 0u ? 0x40u : 0u)
            | (diff & 0x80u)
            | ((((value ^ 0xffu) & (value ^ diff)) & 0x80u) != 0u ? 0x800u : 0u);
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }
    // The callee keeps eax/ecx/esi and leaves the flags of its last add; the mov does not touch flags.
    gp4::call_regs(0x0040fa51, *r);
    GP4_GLOBAL(uint8_t, 0x0095404e) = 0u;
}
GP4_IMPL(0x0040fa3b, FUN_0040fa3b, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=cars_0040fa51; cases=200")
