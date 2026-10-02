#include <gp4/gp4.h>

// FUNCTION: GP4 0x004015dc
void __cdecl FUN_004015dc(gp4::Regs* r) {
    gp4::call_regs(0x004014ba, *r);
}
GP4_IMPL(0x004015dc, FUN_004015dc, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0062d1b4")
