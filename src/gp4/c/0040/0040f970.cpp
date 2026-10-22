#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040f970
// `pushal ; call 0x00443833 ; call 0x0044528b ; mov byte [0x007aafe4], 0 ; popal ; ret`: runs two
// frame-level routines and clears the byte at 0x007aafe4. Every register is restored; the flags
// are those of the last flag writer inside the callees.
void __cdecl FUN_0040f970(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00443833, c);
    gp4::call_regs(0x0044528b, c);
    GP4_GLOBAL(uint8_t, 0x007aafe4) = 0;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0040f970, FUN_0040f970, "regs(ecx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+carcc_0043a785+ring_008943a0+records_008943a0+segw_00441e25+state_00441eb8+state_0040e949+flags_00432dcf+cars_00433d8d+state_00432f18+state_004320f2+lap_0043b482+cars_00443648+cars_004436f9+state_00442d1f+state_0044334d+state_0043b920+cur_0066d81c+cur_00852d30+stream_0062d1a0+stream_0062d1a8+state_0040fa01+state_0041104f+carx_0042fe2f+state_00413f28+send_004453d1")
