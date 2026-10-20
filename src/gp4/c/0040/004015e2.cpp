#include <gp4/gp4.h>

// Tail wrapper around the bounded random-integer routine 0x0040150b: the bound arrives in eax,
// the result (1..eax) and the flags of its final "inc eax" are left for the caller untouched.
// FUNCTION: GP4 0x004015e2
void __cdecl FUN_004015e2(gp4::Regs* r) {
    gp4::call_regs(0x0040150b, *r);
}
GP4_IMPL(0x004015e2, FUN_004015e2, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0062d1b4")
