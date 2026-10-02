#include <gp4/gp4.h>

// FUNCTION: GP4 0x004247fa
void __cdecl FUN_004247fa(gp4::Regs* r) {
    const uint8_t* table = GP4_ARRAY(uint8_t, 0x0063bcfc);
    // scan entries 0x16..0x3f for the first zero byte
    uint32_t index = 0x16u;
    while (index != 0x40u && table[index] != 0u) {
        ++index;
    }
    r->eax = index;
    // zero found: cmp byte, 0 leaves ZF|PF and clc clears CF.
    // none found: the final cmp eax, 0x40 leaves ZF|PF and stc sets CF.
    const uint32_t flags = (index == 0x40u) ? 0x45u : 0x44u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004247fa, FUN_004247fa, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=bytes_0063bcfc")
