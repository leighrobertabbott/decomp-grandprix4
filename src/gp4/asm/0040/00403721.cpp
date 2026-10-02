#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit cmp left, right.
static inline uint32_t cmp32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00403721
void __cdecl FUN_00403721(gp4::Regs* r) {
    const uint32_t position = r->edi;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    const uint32_t value = r->eax;
    uint32_t flags;
    if (position >= limit) {
        // cmp edi, limit ; jae -> mov eax, 1 (mov keeps the flags of the cmp)
        flags = cmp32Flags(position, limit);
        r->eax = 1u;
    } else {
        // cmp eax, 1 ; jne -> ret, otherwise mov eax, 7
        flags = cmp32Flags(value, 1u);
        if (value == 1u) {
            r->eax = 7u;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00403721, FUN_00403721, "regs(eax:i32[0..2], edi:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
