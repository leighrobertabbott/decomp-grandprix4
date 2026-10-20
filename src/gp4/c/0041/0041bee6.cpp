#include <gp4/gp4.h>

// Runs the four per-car update routines in order. Each is a register-convention routine working on the
// car record in esi, so one register block is threaded through all four calls and handed back as the
// wrapper's outputs.
// FUNCTION: GP4 0x0041bee6
void __cdecl FUN_0041bee6(gp4::Regs* r) {
    gp4::Regs regs = *r;
    gp4::call_regs(0x0041be61, regs);
    gp4::call_regs(0x00416457, regs);
    gp4::call_regs(0x0041b852, regs);
    gp4::call_regs(0x0041b950, regs);
    *r = regs;
}
GP4_IMPL(0x0041bee6, FUN_0041bee6, "regs(esi:ptr[0x224]:layout=rec_0041b950) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0041b950")
