#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0043ca4a
void __cdecl FUN_0043ca4a(gp4::Regs* r) {
    const uint32_t end = GP4_GLOBAL(uint32_t, 0x006227e0);
    const uint32_t start = GP4_GLOBAL(uint32_t, 0x006227dc);
    // sub eax, edx ; imul dword [DAT_006227a0] ; shrd eax, edx, 14
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(end - start))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006227a0));
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    // pop edx restores the start value; add eax, edx leaves the final flags
    const uint32_t result = shifted + start;
    const uint32_t flags = (result < shifted ? 1u : 0u)
        | parityFlag(result)
        | (((shifted ^ start ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(shifted ^ start) & (shifted ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eax = result;
    r->edx = start;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ca4a, FUN_0043ca4a, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
