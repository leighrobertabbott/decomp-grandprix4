#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051addd
// Tail wrapper: `call 0x0040838c ; ret` (eax = address of the clamped record of the 0x0066fef0 table).
void __cdecl FUN_0051addd(gp4::Regs* r) {
    gp4::call_regs(0x0040838c, *r);
}
GP4_IMPL(0x0051addd, FUN_0051addd, "regs(eax:i32[-8..40]) -> (eax, cf, pf, af, zf, sf, of)")
