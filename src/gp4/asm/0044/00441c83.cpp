#include <gp4/gp4.h>

// Marks bit eax (0..63) of slot 1 through 0x00434063 (edx = bit, eax = slot). pushal / popal restore every
// register; only the flags left by 0x00434063 survive.
// FUNCTION: GP4 0x00441c83
void __cdecl FUN_00441c83(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edx = r->eax;
    c.eax = 1u;
    gp4::call_regs(0x00434063, c);
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00441c83, FUN_00441c83, "regs(eax:u32[0..63]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
