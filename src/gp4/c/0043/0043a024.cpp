#include <gp4/gp4.h>

struct Unknown_0063bd7c { uint8_t Unknown00[0x74]; int32_t Unknown74; };

// FUNCTION: GP4 0x0043a024
void __cdecl FUN_0043a024(gp4::Regs* r) {
    for (int i = 0; i != 0x40; i++) {
        if (GP4_ARRAY(uint8_t, 0x0063bcfc)[i] != 0) {
            GP4_ARRAY(Unknown_0063bd7c, 0x0063bd7c)[i].Unknown74 = -1;
        }
    }
    for (int i = 0; i != 0x10; i++) {
        GP4_ARRAY(uint8_t, 0x0063db7c)[i] = 0;
    }
    // The original leaves ecx = 0x10, esi = end of the record array and the
    // flags of its final "cmp ecx, 0x10" (equal: ZF and PF set, rest clear).
    r->ecx = 0x10;
    r->esi = 0x0063db7c;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043a024, FUN_0043a024, "regs() -> (ecx, esi, cf, pf, af, zf, sf, of); globals=layout=bytes_0063bcfc")
