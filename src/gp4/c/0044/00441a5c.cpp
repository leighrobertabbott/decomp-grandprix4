#include <gp4/gp4.h>

// pushal wrapper: runs the control-block / segment-record setup (0x00441a69) and then switches the groups off with
// the word-sized loop 0x00441df7. popal restores every register; the flags are those 0x00441df7 leaves.
// FUNCTION: GP4 0x00441a5c
void __cdecl FUN_00441a5c(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00441a69, call);
    gp4::call_regs(0x00441df7, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00441a5c, FUN_00441a5c, "regs(eax:u32, edx:u32, edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+segw_00441e25+words_008943a0+ptr_0086850c")
