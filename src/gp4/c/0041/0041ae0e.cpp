#include <gp4/gp4.h>

// Tail wrapper: jumps through the routine at 0x0041adbd with the caller's registers (esi = body
// block) and leaves its eax result and flags untouched.
// FUNCTION: GP4 0x0041ae0e
void __cdecl FUN_0041ae0e(gp4::Regs* r) {
    gp4::call_regs(0x0041adbd, *r);
}
GP4_IMPL(0x0041ae0e, FUN_0041ae0e, "regs(esi:ptr[0x1d5]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
