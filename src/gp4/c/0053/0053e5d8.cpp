#include <gp4/gp4.h>
#include <gp4/flags.h>

// push edx / edx = 0 / call 0x0053e5e5 / pop edx: the 64-bit mask edx:eax = eax:0 is consumed one
// bit per node along the list at esi by 0x0053e5e5 (pushal/popal). edx comes back unchanged and
// so does every other register; the flags are those 0x0053e5e5 leaves.
// FUNCTION: GP4 0x0053e5d8
void __cdecl FUN_0053e5d8(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edx = 0u;
    gp4::call_regs(0x0053e5e5, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053e5d8, FUN_0053e5d8, "regs(eax:u32, esi:u32{=0x84f066}) -> (cf, pf, af, zf, sf, of); globals=layout=session_004fd1a0+flags_0084f066")
