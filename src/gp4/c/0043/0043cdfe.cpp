#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043cdfe
void __cdecl FUN_0043cdfe(gp4::Regs* r) {
    gp4::call_regs(0x0043cde2, *r);
}
GP4_IMPL(0x0043cdfe, FUN_0043cdfe, "regs(eax:u32[0..4]) -> (eax, cf, pf, af, zf, sf, of)")
