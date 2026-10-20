#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040c3a1
// Tail wrapper: `call 0x0040c3a7 ; ret` (clamps eax to 0..11 into DAT_00622584; eax and the
// flags come straight back from the callee).
void __cdecl FUN_0040c3a1(gp4::Regs* r) {
    gp4::call_regs(0x0040c3a7, *r);
}
GP4_IMPL(0x0040c3a1, FUN_0040c3a1, "regs(eax:i32[-4..20]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
