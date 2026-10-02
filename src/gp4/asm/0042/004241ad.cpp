#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x004241ad
void __cdecl FUN_004241ad(gp4::Regs* r) {
    const uint32_t ebx = r->ebx;
    // cmp ebx, 1 sets the flags; when ebx == 1 the later cmp of DAT_00608304 replaces them
    uint32_t lhs = ebx;
    uint32_t rhs = 1u;
    if (ebx == 1u) {
        lhs = GP4_GLOBAL(uint32_t, 0x00608304);
        rhs = 0x3a000000u;
        // jl not taken: the original loads [esi + ebx*4] and discards it (eax is restored)
    }
    const uint32_t result = lhs - rhs;
    const uint32_t flags = (lhs < rhs ? 1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004241ad, FUN_004241ad, "regs(ebx:u32[0..3], esi:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
