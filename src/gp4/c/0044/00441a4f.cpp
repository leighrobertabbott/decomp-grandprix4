#include <gp4/gp4.h>

// pushal wrapper: switches the groups off (0x00441de4) and then clears the transient state of the ring record at the
// lap start (0x00441ad7). popal restores every register; the flags are those 0x00441ad7 leaves.
// FUNCTION: GP4 0x00441a4f
void __cdecl FUN_00441a4f(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00441de4, call);
    gp4::call_regs(0x00441ad7, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00441a4f, FUN_00441a4f, "regs(edx:u32, edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+segw_00441e25")
