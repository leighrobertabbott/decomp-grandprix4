#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043cdf8
void __cdecl FUN_0043cdf8(gp4::Regs* r) {
    gp4::call_regs(0x0043cdcc, *r);
}
GP4_IMPL(0x0043cdf8, FUN_0043cdf8, "regs(eax:u32[0..4]) -> (eax, cf, pf, af, zf, sf, of)")
