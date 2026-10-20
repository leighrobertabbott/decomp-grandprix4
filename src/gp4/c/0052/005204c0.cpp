#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the row-bound tables and the fill buffer (0x005204d0), clears the row-slot state
// (0x005205cf) and reloads the per-index tables (0x00521ce9). All three are pushal/popal routines, so
// every register comes back unchanged; the flags thread through the three calls and the last writer's
// remain.
// FUNCTION: GP4 0x005204c0
void __cdecl FUN_005204c0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x005204d0, c);
    gp4::call_regs(0x005205cf, c);
    gp4::call_regs(0x00521ce9, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005204c0, FUN_005204c0, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rows_00520584+fill_005204d0")
