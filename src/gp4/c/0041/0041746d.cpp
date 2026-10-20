#include <gp4/gp4.h>

// Tail wrapper: `call 0x0041743e ; ret`. Registers and flags come straight back from the callee.
// FUNCTION: GP4 0x0041746d
void __cdecl FUN_0041746d(gp4::Regs* r) {
    gp4::call_regs(0x0041743e, *r);
}
GP4_IMPL(0x0041746d, FUN_0041746d, "regs(esi:ptr[0x300]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
