#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the texture-slot copier 0x0051c894 for the four slots 0x43d, 0x43f, 0x43e and 0x440 (in that
// order; ebx carries the slot number). The copier restores every register, so only ebx = 0x440 and
// the flags of the last copier call remain.
// FUNCTION: GP4 0x0051c8c0
void __cdecl FUN_0051c8c0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = 0x43du;
    gp4::call_regs(0x0051c894, c);
    c.ebx = 0x43fu;
    gp4::call_regs(0x0051c894, c);
    c.ebx = 0x43eu;
    gp4::call_regs(0x0051c894, c);
    c.ebx = 0x440u;
    gp4::call_regs(0x0051c894, c);
    r->ebx = 0x440u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051c8c0, FUN_0051c8c0, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=args_0051c865+slots_0051c894")
