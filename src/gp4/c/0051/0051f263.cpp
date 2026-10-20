#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; call 0x00518545; popal; ret
// The callee gets the entry registers; popal then restores every general register to its entry
// value but leaves the flags of the callee's last flag writer.
// FUNCTION: GP4 0x0051f263
void __cdecl FUN_0051f263(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00518545, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f263, FUN_0051f263, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=full_00518b09+chan_005188f5+chan_00518654+snd_00518758+sndsel_00518545")
