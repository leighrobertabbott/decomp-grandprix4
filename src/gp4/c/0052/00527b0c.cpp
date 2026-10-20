#include <gp4/gp4.h>
#include <gp4/flags.h>

// call 0x005ae990 (a bare `ret`); pushal; call 0x004ff070; popal; call 0x005ae990; ret
// The compiled callee 0x004ff070 gets the entry registers; popal then restores every general
// register to its entry value but leaves the flags of the callee's last flag writer. The two calls
// to the empty routine 0x005ae990 change nothing.
// FUNCTION: GP4 0x00527b0c
void __cdecl FUN_00527b0c(gp4::Regs* r) {
    gp4::cdecl_<0x005ae990, void>();
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x004ff070, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
    gp4::cdecl_<0x005ae990, void>();
}
GP4_IMPL(0x00527b0c, FUN_00527b0c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
