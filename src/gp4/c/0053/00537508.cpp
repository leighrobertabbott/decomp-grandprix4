#include <gp4/gp4.h>

// Runs the four memory-block routines in order (0x0053754c, 0x005375f0, 0x0053751d, 0x005376aa).
// Each is pushal / popal wrapped, so one register block is threaded through the calls and only the
// flags of the last one remain visible.
// FUNCTION: GP4 0x00537508
void __cdecl FUN_00537508(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0053754c, c);
    gp4::call_regs(0x005375f0, c);
    gp4::call_regs(0x0053751d, c);
    gp4::call_regs(0x005376aa, c);
    *r = c;
}
GP4_IMPL(0x00537508, FUN_00537508, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
