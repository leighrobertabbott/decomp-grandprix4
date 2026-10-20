#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043ba31
// Walks every driver (esi = 0..21) and circuit (ebx = 0..16): 0x0043ba12 clears the 'I' marked byte
// of the pair, then for the four 0x38-byte entries (ecx = 0..3) of that pair 0x0051f837 returns the
// entry address (edi) and 0x0043b9c5 fills the high bytes of its words +0x2e / +0x2c. pushal/popal
// restore every register; popal keeps the flags of the last `cmp esi, 0x16` (equal: ZF and PF).
void __cdecl FUN_0043ba31(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    for (c.esi = 0; c.esi != 0x16u; ++c.esi) {
        for (c.ebx = 0; c.ebx != 0x11u; ++c.ebx) {
            gp4::call_regs(0x0043ba12, c);
            for (c.ecx = 0; c.ecx != 4u; ++c.ecx) {
                c.eax = c.esi;
                gp4::call_regs(0x0051f837, c);
                c.edi = c.eax;
                gp4::call_regs(0x0043b9c5, c);
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043ba31, FUN_0043ba31, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=codes_0043ba12+opts_0043ba31")
