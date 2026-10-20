#include <gp4/gp4.h>

// pushal / mov esi,"Not Allocated\n" / call 0x005373bf / popal / ret: calls the message routine
// 0x005373bf with esi = the string at 0x00829b24. popal restores every register; the flags are
// those 0x005373bf leaves.
// FUNCTION: GP4 0x00537b73
void __cdecl FUN_00537b73(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.esi = 0x00829b24u;
    gp4::call_regs(0x005373bf, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00537b73, FUN_00537b73, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
