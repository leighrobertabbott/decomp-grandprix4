#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043edc1
void __cdecl FUN_0043edc1(gp4::Regs* r) {
    // First test (0x43ed97); carry set -> stc and return with its flags.
    gp4::call_regs(0x0043ed97, *r);
    if ((r->eflags & gp4::CF) != 0u) {
        return;
    }
    // Second test (0x43ed48): stc/clc just restate its carry, so its flags are the result.
    gp4::call_regs(0x0043ed48, *r);
}
GP4_IMPL(0x0043edc1, FUN_0043edc1, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0043ed48")
