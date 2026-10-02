#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0041c00c
void __cdecl FUN_0041c00c(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00623b90);
    GP4_GLOBAL(uint32_t, 0x00623b94) = value;
    GP4_GLOBAL(uint32_t, 0x006380dc) = value;       // [ebx + 4]
    const uint32_t negated = 0u - value;
    GP4_GLOBAL(uint32_t, 0x006383dc) = negated;     // [ebx + 0x304]

    // neg eax: CF = (value != 0), OF = (value == 0x80000000), AF from 0 - value
    const uint32_t flags = (value != 0u ? 1u : 0u)
        | parityFlag(negated)
        | (((value ^ negated) & 0x10u) != 0u ? 0x10u : 0u)
        | (negated == 0u ? 0x40u : 0u)
        | ((negated & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((value & negated & 0x80000000u) != 0u ? 0x800u : 0u);

    r->ebx = 0x006380d8u;
    r->eax = negated;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041c00c, FUN_0041c00c, "regs() -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
