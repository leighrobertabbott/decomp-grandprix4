#include <gp4/gp4.h>

// Tail wrapper: "mov ax, 0x24; call 0x00531f9c; ret". The upper half of eax is untouched; the
// dispatcher's eax and flags are the result.
// FUNCTION: GP4 0x00513be7
void __cdecl FUN_00513be7(gp4::Regs* r) {
    r->eax = (r->eax & 0xffff0000u) | 0x24u;
    gp4::call_regs(0x00531f9c, *r);
}
GP4_IMPL(0x00513be7, FUN_00513be7, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=captions_0052e131")
