#include <gp4/gp4.h>

// FUNCTION: GP4 0x0052a1f6
// Tail wrapper: `call 0x00410c70 ; ret` (7-byte ramp from [eax] to [eax+5], written at edx).
void __cdecl FUN_0052a1f6(gp4::Regs* r) {
    gp4::call_regs(0x00410c70, *r);
}
GP4_IMPL(0x0052a1f6, FUN_0052a1f6, "regs(eax:ptr[0x10]:bytes, edx:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of)")
