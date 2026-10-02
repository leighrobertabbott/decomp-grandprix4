#include <gp4/gp4.h>

// FUNCTION: GP4 0x004247e5
void __cdecl FUN_004247e5(gp4::Regs* r) {
    // clears the 64-byte table at 0x0063bcfc (ecx is saved and restored)
    uint8_t* table = GP4_ARRAY(uint8_t, 0x0063bcfc);
    for (uint32_t i = 0; i != 0x40u; i++) {
        table[i] = 0;
    }
    // the loop's final "cmp ecx, 0x40" leaves ZF|PF
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004247e5, FUN_004247e5, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=bytes_0063bcfc")
