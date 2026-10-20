#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; call 0x004fed30; popal; ret
// The callee gets the entry registers; popal then restores every general register to its entry
// value but leaves the flags of the callee's last flag writer.
// FUNCTION: GP4 0x00548c47
void __cdecl FUN_00548c47(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x004fed30, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00548c47, FUN_00548c47, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
