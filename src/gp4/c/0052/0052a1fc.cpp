#include <gp4/gp4.h>

// FUNCTION: GP4 0x0052a1fc
// Tail wrapper: `call 0x00410cad ; ret` (6-byte ramp from [eax] to [eax+6], written at edx).
void __cdecl FUN_0052a1fc(gp4::Regs* r) {
    gp4::call_regs(0x00410cad, *r);
}
GP4_IMPL(0x0052a1fc, FUN_0052a1fc, "regs(eax:ptr[0x10]:bytes, edx:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of)")
