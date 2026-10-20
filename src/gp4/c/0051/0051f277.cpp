#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; call 0x00518d18; popal; ret
// The callee (the sound-channel scan) gets the entry registers; popal then restores every general register to
// its entry value but leaves the flags of the callee's last flag writer.
// FUNCTION: GP4 0x0051f277
void __cdecl FUN_0051f277(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x00518d18, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f277, FUN_0051f277, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=full_00518b09+snd_0095404c")
