#include <gp4/gp4.h>
#include <gp4/flags.h>

// Saves every general register and the four segment registers, runs 0x005ae9a0 (`mov al, 1 ; ret`) and
// restores them all: no register, flag or memory effect remains.
// FUNCTION: GP4 0x0051887c
void __cdecl FUN_0051887c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x005ae9a0, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051887c, FUN_0051887c, "regs() -> (); globals=fuzz:i32")
