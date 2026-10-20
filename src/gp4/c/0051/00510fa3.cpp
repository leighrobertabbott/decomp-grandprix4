#include <gp4/gp4.h>

// Tail wrapper: "mov ax, 0x4f; call 0x00531f9c; ret". The upper half of eax is untouched; the
// dispatcher's eax and flags are the result. The command 0x4f handler reads ecx and esi.
// FUNCTION: GP4 0x00510fa3
void __cdecl FUN_00510fa3(gp4::Regs* r) {
    r->eax = (r->eax & 0xffff0000u) | 0x4fu;
    gp4::call_regs(0x00531f9c, *r);
}
GP4_IMPL(0x00510fa3, FUN_00510fa3, "regs(eax:u32, ecx:u32, esi:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
