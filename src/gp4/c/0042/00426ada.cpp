#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the 32-bit `left - right` (cmp).
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Carry-flag predicate: edi lies inside the window [DAT_00891500, DAT_00866ed0 - 0x840) while
// DAT_007ad894 == 9. eax is saved and restored; CF is the answer (stc / clc), the other
// arithmetic flags are those of the last comparison executed.
// FUNCTION: GP4 0x00426ada
void __cdecl FUN_00426ada(gp4::Regs* r) {
    const uint32_t ptr = r->edi;
    uint32_t flags;
    bool inside = false;

    const uint32_t mode = GP4_GLOBAL(uint32_t, 0x007ad894);
    flags = cmpFlags(mode, 9u);
    if (mode == 9u) {
        const uint32_t low = GP4_GLOBAL(uint32_t, 0x00891500);
        flags = cmpFlags(ptr, low);
        if (ptr >= low) {
            const uint32_t high = GP4_GLOBAL(uint32_t, 0x00866ed0) - 0x840u;
            flags = cmpFlags(ptr, high);
            if (ptr < high) {
                inside = true;
            }
        }
    }

    flags &= ~1u;
    if (inside) {
        flags |= 1u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426ada, FUN_00426ada, "regs(edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
