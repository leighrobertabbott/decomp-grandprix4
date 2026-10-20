#include <gp4/gp4.h>

// Tail wrapper: `mov eax, 6 ; call 0x00528623 ; ret`. All registers and the flags come back from the
// callee.
// FUNCTION: GP4 0x005337ca
void __cdecl FUN_005337ca(gp4::Regs* r) {
    r->eax = 6u;
    gp4::call_regs(0x00528623, *r);
}
GP4_IMPL(0x005337ca, FUN_005337ca, "regs() -> (eax, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_007aafd3")
