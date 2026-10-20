#include <gp4/gp4.h>

// pushal wrapper: runs 0x00441706 and then 0x00441606 on the record in esi. popal restores every register; the flags
// are those 0x00441606 leaves.
// FUNCTION: GP4 0x0044174b
void __cdecl FUN_0044174b(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00441706, call);
    gp4::call_regs(0x00441606, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0044174b, FUN_0044174b, "regs(esi:ptr[0x84]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
