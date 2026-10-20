#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051f1d4
// Tail wrapper: `call 0x00520023 ; ret` (the callee answers in CF).
void __cdecl FUN_0051f1d4(gp4::Regs* r) {
    gp4::call_regs(0x00520023, *r);
}
GP4_IMPL(0x0051f1d4, FUN_0051f1d4, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=neg22_00520023")
