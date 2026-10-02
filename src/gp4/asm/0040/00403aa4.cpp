#include <gp4/gp4.h>

static inline uint32_t comparisonFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00403aa4
void __cdecl FUN_00403aa4(gp4::Regs* r) {
    const uint32_t value = r->edi;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    // cmp edi, [limit]; jb -> 0 else 1 (the movs leave the flags alone)
    r->eax = value < limit ? 0u : 1u;
    r->eflags = (r->eflags & ~0x8d5u) | comparisonFlags32(value, limit);
}
GP4_IMPL(0x00403aa4, FUN_00403aa4, "regs(edi:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
