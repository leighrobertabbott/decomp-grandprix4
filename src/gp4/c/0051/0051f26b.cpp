#include <gp4/gp4.h>

// `call 0x0051850a ; ret`: tail wrapper. The callee starts a sound-channel entry from the sample
// parameters 0x0088f6cc.. (it saves and restores eax..edx itself and runs the pushal/popal routine
// 0x00518758), so every register comes back unchanged; the flags are what the callee leaves (the
// `test [0x95404c]` when the sound system is off, otherwise the carry answer of 0x00518758).
// FUNCTION: GP4 0x0051f26b
void __cdecl FUN_0051f26b(gp4::Regs* r) {
    gp4::call_regs(0x0051850a, *r);
}
GP4_IMPL(0x0051f26b, FUN_0051f26b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=full_00518b09+chan_005188f5+chan_00518654+snd_00518758")
