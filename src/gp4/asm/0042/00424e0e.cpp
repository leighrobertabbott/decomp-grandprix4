#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00424e0e
void __cdecl FUN_00424e0e(gp4::Regs* r) {
    const uint32_t index = r->eax;
    const uint8_t value = GP4_ARRAY(uint8_t, 0x0063bcfc)[index];
    // cmp byte [eax + 0x63bcfc], 0 gives CF=AF=OF=0 and ZF/SF/PF from the byte;
    // stc (nonzero) or clc (zero) then sets CF to the nonzero test.
    const uint32_t flags = (value != 0u ? 1u : 0u)
        | parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00424e0e, FUN_00424e0e, "regs(eax:u32[0..63]) -> (cf, pf, af, zf, sf, of); globals=layout=bytes_0063bcfc")
