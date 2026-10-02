#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00406a43
void __cdecl FUN_00406a43(gp4::Regs* r) {
    // edi -= 0x160; carry set when the lowered edi is below [0x866ed0] (unsigned)
    const uint32_t left = r->edi - 0x160u;
    const uint32_t right = GP4_GLOBAL(uint32_t, 0x00866ed0);
    const uint32_t result = left - right;
    // cmp flags; the following stc/clc just restates CF = left < right
    const uint32_t flags = parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
    r->edi = left;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00406a43, FUN_00406a43, "regs(edi:u32) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
