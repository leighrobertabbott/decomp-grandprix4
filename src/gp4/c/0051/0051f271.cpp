#include <gp4/gp4.h>

// `call 0x00518538 ; ret`: tail wrapper. The callee loads eax = [0x0088f6cc] (the current sample) and
// releases its channel through the pushal/popal routine 0x00518816; eax is saved and restored, so
// every register comes back unchanged and the flags are the callee's (clc / stc plus the flags of
// its last compare).
// FUNCTION: GP4 0x0051f271
void __cdecl FUN_0051f271(gp4::Regs* r) {
    gp4::call_regs(0x00518538, *r);
}
GP4_IMPL(0x0051f271, FUN_0051f271, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=full_00518b09+chan_005188f5+chan_00518654+snd_00518758")
