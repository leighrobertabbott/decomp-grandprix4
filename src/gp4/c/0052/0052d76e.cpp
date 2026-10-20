#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tail wrapper: `mov al, 0x1b ; call 0x0052d738 ; ret`. Tests the bitmap bit of key code 0x1b (byte 3,
// bit 3 of 0x008245a0); only al changes in eax, the flags are those of the callee's `and`.
// FUNCTION: GP4 0x0052d76e
void __cdecl FUN_0052d76e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (r->eax & 0xffffff00u) | 0x1bu;
    gp4::call_regs(0x0052d738, c);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052d76e, FUN_0052d76e, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=bits_00510e2d")
