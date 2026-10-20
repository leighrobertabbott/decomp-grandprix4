#include <gp4/gp4.h>

// `call 0x0054ba6b ; clc ; ret`: advances the 3-bit counter byte at 0x007aafd0 through al (the
// callee leaves the new value in al and the flags of `and al, 7`), then clears the carry flag.
// FUNCTION: GP4 0x0054a484
void __cdecl FUN_0054a484(gp4::Regs* r) {
    gp4::call_regs(0x0054ba6b, *r);
    r->eflags &= ~1u;
}
GP4_IMPL(0x0054a484, FUN_0054a484, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ctr_0054a678")
