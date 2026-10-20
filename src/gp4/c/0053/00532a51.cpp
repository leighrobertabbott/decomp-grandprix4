#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests the key-bitmap bit of key code 0x1b (`mov al, 0x1b ; call 0x0052d738`) and returns
// eax = 0 when ZF is set (bit clear), else eax = -1. The flags stay those of the callee's `and`.
// FUNCTION: GP4 0x00532a51
void __cdecl FUN_00532a51(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (r->eax & 0xffffff00u) | 0x1bu;
    gp4::call_regs(0x0052d738, c);
    r->eax = (c.eflags & gp4::flags::ZF) ? 0u : 0xffffffffu;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00532a51, FUN_00532a51, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=bits_00510e2d")
