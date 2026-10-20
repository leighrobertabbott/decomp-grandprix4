#include <gp4/gp4.h>

// One draw from the generator at 0x401462, then the four-draw routine 0x43d158. pushal/popal
// restore every register, so only the generator state and the flags remain; popal does not
// restore the flags, which are those 0x43d158 leaves (its final scaling shift).
// FUNCTION: GP4 0x0043d187
void __cdecl FUN_0043d187(gp4::Regs* r) {
    gp4::Regs call = *r;
    gp4::call_regs(0x00401462, call);
    gp4::call_regs(0x0043d158, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0043d187, FUN_0043d187, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
