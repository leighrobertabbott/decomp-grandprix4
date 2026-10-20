#include <gp4/gp4.h>
#include <gp4/flags.h>

// `push eax; mov eax,[0x0088f6cc]; call 0x00518816; pop eax; ret`: hands the current sample index
// [0x0088f6cc] to the sound-channel release routine 0x00518816 (a pushal/popal routine that ends in
// clc/stc). eax is restored by the pop; the flags are those the callee leaves (its stc/clc epilogue
// replaces only CF).
// FUNCTION: GP4 0x00518538
void __cdecl FUN_00518538(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = GP4_GLOBAL(uint32_t, 0x0088f6cc);
    gp4::call_regs(0x00518816, call);
    r->eflags = gp4::flags::merge(r->eflags, call.eflags);
}
GP4_IMPL(0x00518538, FUN_00518538, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=chan_005188f5+chan_00518654+rel_00518816")
