#include <gp4/gp4.h>

// "mov eax, 0x3e; mov ecx, 0; call 0x00531f9c; ret": dispatcher command 0x3e with ecx = 0. eax and
// the flags are the handler's; ecx stays 0 (the dispatcher saves and restores it).
// FUNCTION: GP4 0x0051689c
void __cdecl FUN_0051689c(gp4::Regs* r) {
    r->eax = 0x3eu;
    r->ecx = 0u;
    gp4::call_regs(0x00531f9c, *r);
}
GP4_IMPL(0x0051689c, FUN_0051689c, "regs() -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
