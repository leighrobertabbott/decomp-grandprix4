#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// Arithmetic flags of `left - right` (also `cmp left, right`).
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Advances the address in eax by edx and wraps it inside one of two ring buffers: the one that
// ends at DAT_00866ef8 / restarts at DAT_00891500 (for addresses at or above DAT_00891500), or
// the one that ends at DAT_00866ecc / restarts at 0x008943a0 (below it). The flags are those of
// the last arithmetic instruction executed. (The "test byte [esi+0x71], 0x80" is dead: both
// outcomes continue at the next instruction.)
// FUNCTION: GP4 0x00429339
void __cdecl FUN_00429339(gp4::Regs* r) {
    uint32_t value = r->eax;
    const uint32_t delta = r->edx;
    uint32_t flags;

    if (value >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        uint32_t left = value;
        value += delta;
        flags = addFlags(left, delta, value);
        if (value < GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            flags = subFlags(value, GP4_GLOBAL(uint32_t, 0x00891500));
            if (value < GP4_GLOBAL(uint32_t, 0x00891500)) {
                left = value;
                value -= GP4_GLOBAL(uint32_t, 0x00891500);
                flags = subFlags(left, GP4_GLOBAL(uint32_t, 0x00891500));
                left = value;
                value += GP4_GLOBAL(uint32_t, 0x00866f00);
                flags = addFlags(left, GP4_GLOBAL(uint32_t, 0x00866f00), value);
            }
        } else {
            left = value;
            value -= GP4_GLOBAL(uint32_t, 0x00866ef8);
            flags = subFlags(left, GP4_GLOBAL(uint32_t, 0x00866ef8));
            left = value;
            value += GP4_GLOBAL(uint32_t, 0x00866f04);
            flags = addFlags(left, GP4_GLOBAL(uint32_t, 0x00866f04), value);
        }
    } else {
        uint32_t left = value;
        value += delta;
        flags = addFlags(left, delta, value);
        if (value < GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            flags = subFlags(value, 0x008943a0u);
            if (value < 0x008943a0u) {
                left = value;
                value += GP4_GLOBAL(uint32_t, 0x00866ec4);
                flags = addFlags(left, GP4_GLOBAL(uint32_t, 0x00866ec4), value);
            }
        } else {
            flags = subFlags(value, GP4_GLOBAL(uint32_t, 0x00866ec4));
            value -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    }

    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00429339, FUN_00429339, "regs(eax:u32, edx:u32, esi:ptr[0x80]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
