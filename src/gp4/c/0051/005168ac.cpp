#include <gp4/gp4.h>

// "mov eax, 0x3e; mov ecx, 1; call 0x00531f9c; ret": dispatcher command 0x3e with ecx = 1. eax and
// the flags are the handler's; ecx stays 1 (the dispatcher saves and restores it).
// FUNCTION: GP4 0x005168ac
void __cdecl FUN_005168ac(gp4::Regs* r) {
    r->eax = 0x3eu;
    r->ecx = 1u;
    gp4::call_regs(0x00531f9c, *r);
}
GP4_IMPL(0x005168ac, FUN_005168ac, "regs() -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
