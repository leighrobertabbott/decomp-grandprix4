#include <gp4/gp4.h>

// Tail wrapper: "mov ax, 0x25; call 0x00531f9c; ret". The upper half of eax is untouched; the
// dispatcher's eax and flags are the result. The command 0x25 handler expands the string at esi
// into the buffer at edi.
// FUNCTION: GP4 0x00513c05
void __cdecl FUN_00513c05(gp4::Regs* r) {
    r->eax = (r->eax & 0xffff0000u) | 0x25u;
    gp4::call_regs(0x00531f9c, *r);
}
GP4_IMPL(0x00513c05, FUN_00513c05, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=save_0053ddbb+text_005325bc")
