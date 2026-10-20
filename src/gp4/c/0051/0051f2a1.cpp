#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; call 0x00510fc1; popal; ret
// The callee gets the entry registers; popal then restores every general register to its entry
// value but leaves the flags of the callee's last flag writer.
// FUNCTION: GP4 0x0051f2a1
void __cdecl FUN_0051f2a1(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00510fc1, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f2a1, FUN_0051f2a1, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=fill_00541fad")
