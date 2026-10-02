#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) left by `sub left, right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004060e6
void __cdecl FUN_004060e6(gp4::Regs* r) {
    const uint32_t entry = r->edi;
    const uint32_t boundary = GP4_GLOBAL(uint32_t, 0x00891500);
    uint32_t value = entry;

    if (entry < boundary) {
        if (entry == 0x008943a0u) {
            value = GP4_GLOBAL(uint32_t, 0x00866ecc);
        } else if (entry == GP4_GLOBAL(uint32_t, 0x00866f04)) {
            value = GP4_GLOBAL(uint32_t, 0x00866ef8);
        }
    } else if (entry == boundary) {
        value = GP4_GLOBAL(uint32_t, 0x00866f00);
    }

    // sub edi, 0x160 leaves the flags
    r->edi = value - 0x160u;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags(value, 0x160u);
}
GP4_IMPL(0x004060e6, FUN_004060e6, "regs(edi:u32) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
