#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; call 0x0051a78d; popal; ret
// The callee gets the entry registers; popal then restores every general register to its entry
// value but leaves the flags of the callee's last flag writer.
// FUNCTION: GP4 0x0051f1f7
void __cdecl FUN_0051f1f7(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0051a78d, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f1f7, FUN_0051f1f7, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+list_0066fe3c+full_00518b09+chan_005188f5+chan_00518654+snd_00518758+state_00418d27+order_00418f8f+player_0040f8d9+box_00550798+sndrec_0051a78d+sndgate_0051a78d")
