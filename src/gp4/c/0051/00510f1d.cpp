#include <gp4/gp4.h>

// "mov byte [0x007a9792], 0xff; mov ax, 0x49; call 0x00531f9c; ret": marks 0x007a9792, then runs
// dispatcher command 0x49. The upper half of eax is untouched; eax and the flags are the dispatcher's.
// FUNCTION: GP4 0x00510f1d
void __cdecl FUN_00510f1d(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x007a9792) = 0xffu;
    r->eax = (r->eax & 0xffff0000u) | 0x49u;
    gp4::call_regs(0x00531f9c, *r);
}
GP4_IMPL(0x00510f1d, FUN_00510f1d, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=dw_00532d48")
