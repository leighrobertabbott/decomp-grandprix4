#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
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

// FUNCTION: GP4 0x00434044
void __cdecl FUN_00434044(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->esi;
    const uint32_t slot = r->eax;

    int32_t bit = (int32_t)(GP4_FIELD(uint8_t, body, 0x7c) & 0x3fu) - 1;
    const uint32_t cmpFlags = subtractionFlags32((uint32_t)bit, 0x15u);   // cmp edx, 0x15
    if (bit > 0x15) {
        bit = 0x15;
    }

    // bts dword [eax*4 + 0x62ff6c], edx: a register bit offset addresses
    // dword (bit >> 5) relative to the operand, so bit = -1 reaches the dword before it.
    uint32_t* word = &GP4_ARRAY(uint32_t, 0x0062ff6c)[slot + (uint32_t)(bit >> 5)];
    const uint32_t mask = 1u << (bit & 31);
    const uint32_t previous = (*word & mask) != 0u ? 1u : 0u;
    *word |= mask;

    // bts replaces CF with the old bit and leaves the cmp result's other flags.
    r->eflags = (r->eflags & ~0x8d5u) | (cmpFlags & ~1u) | previous;
}
GP4_IMPL(0x00434044, FUN_00434044, "regs(esi:ptr[0x84]:bytes, eax:u32[0..31]) -> (cf, pf, af, zf, sf, of); globals=layout=rows_00434044")
