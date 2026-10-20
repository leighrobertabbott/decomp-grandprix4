#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / push ds,es,fs,gs wrapper around 0x0050a070 (which calls the virtual method at +4 of the
// object [0x00b1d3dc], if set, and returns al = 1). Every register is restored; popal does not
// restore the flags, so those of the callee remain.
// FUNCTION: GP4 0x00518868
void __cdecl FUN_00518868(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0050a070, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00518868, FUN_00518868, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
