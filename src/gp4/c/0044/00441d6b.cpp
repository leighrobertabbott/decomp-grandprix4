#include <gp4/gp4.h>

// Calls 0x00441d85 (marks or unmarks the 32 consecutive segment records from the lap start) with dh = 1, i.e. in
// clear mode, and ebx = 0x0066fd70. pushal / popal restore every register; only the flags of the call remain.
// FUNCTION: GP4 0x00441d6b
void __cdecl FUN_00441d6b(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ebx = 0x0066fd70u;
    call.edx = (r->edx & 0xffff00ffu) | 0x100u;
    gp4::call_regs(0x00441d85, call);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00441d6b, FUN_00441d6b, "regs(ecx:u32[0..1]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+w1a_008943a0")
