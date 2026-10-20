#include <gp4/gp4.h>

// pushal wrapper around 0x00441e25 with esi = 0 (switch the group off). popal restores every register; the flags
// are those 0x00441e25 leaves.
// FUNCTION: GP4 0x00441e18
void __cdecl FUN_00441e18(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.esi = 0u;
    gp4::call_regs(0x00441e25, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00441e18, FUN_00441e18, "regs(eax:u32[0..1], edx:u32, edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+segw_00441e25")
