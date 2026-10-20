#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: loads the drawing globals from the segment record edi (0x00551155), then fills the
// first point row (esi = 0) from the same record with 0x00554ab1. popal restores every register; the
// flags are those left by 0x00554ab1.
// FUNCTION: GP4 0x00554c40
void __cdecl FUN_00554c40(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x00551155, c);
    c.esi = 0u;
    gp4::call_regs(0x00554ab1, c);
    r->eflags = c.eflags;
}
GP4_IMPL(0x00554c40, FUN_00554c40, "regs(edi:rec[0x8943a0,0x160,0..7]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=geom_008943a0+span_00551227+w6_008943a0+words_008943a0")
