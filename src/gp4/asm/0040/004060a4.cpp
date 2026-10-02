#include <gp4/gp4.h>

// FUNCTION: GP4 0x004060a4
void __cdecl FUN_004060a4(gp4::Regs* r) {
    // add edi, 0x160, then fall into the 0x4060b0 routine (its cmp sets every flag)
    r->edi += 0x160u;
    gp4::call_regs(0x004060b0, *r);
}
GP4_IMPL(0x004060a4, FUN_004060a4, "regs(edi:u32) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
