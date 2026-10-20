#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: `push [0x007ad894]; call 0x004ff760; popal`. The current circuit number
// [0x007ad894] goes to 0x004ff760 (stdcall, one argument, fills the circuit text controls). popal
// restores every register but not the flags, which are those 0x004ff760 leaves.
// FUNCTION: GP4 0x0053af99
void __cdecl FUN_0053af99(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    const uint32_t args[1] = {GP4_GLOBAL(uint32_t, 0x007ad894)};
    gp4::call_regs_stdcall(0x004ff760, c, args);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053af99, FUN_0053af99, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=trk_007ad894+circ_007c3ae8+units_007ad8ac+cname_00838236+strs_004ff7a0+script_0053afa7+script_0052ad37+laprec_007ab760")
