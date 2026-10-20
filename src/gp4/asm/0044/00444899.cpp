#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Stores the three register arguments (eax, edx, ecx) into 0x0062a044 / 0x0062a048 / 0x0062a04c; the flags
// are those of the final `cmp ecx, 0x400` (both jl directions reach the same ret).
// FUNCTION: GP4 0x00444899
void __cdecl FUN_00444899(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0062a044) = r->eax;
    GP4_GLOBAL(uint32_t, 0x0062a048) = r->edx;
    GP4_GLOBAL(uint32_t, 0x0062a04c) = r->ecx;

    const uint32_t left = r->ecx;
    const uint32_t right = 0x400u;
    const uint32_t result = left - right;
    const uint32_t flags = (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00444899, FUN_00444899, "regs(eax:u32, ecx:u32, edx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
