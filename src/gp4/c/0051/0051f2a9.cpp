#include <gp4/gp4.h>

// Tail wrapper: "call 0x00510f1d; ret". The registers and flags are 0x00510f1d's.
// FUNCTION: GP4 0x0051f2a9
void __cdecl FUN_0051f2a9(gp4::Regs* r) {
    gp4::call_regs(0x00510f1d, *r);
}
GP4_IMPL(0x0051f2a9, FUN_0051f2a9, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=dw_00532d48")
