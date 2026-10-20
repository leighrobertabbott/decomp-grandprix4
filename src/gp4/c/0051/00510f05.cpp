#include <gp4/gp4.h>

// "push esi; mov byte [0x007a9792], 0xff; mov esi, 0x7c595c; mov ax, 0x48; call 0x00531f9c; pop esi; ret":
// marks 0x007a9792, then runs dispatcher command 0x48 with esi pointing at the table 0x007c595c. The
// upper half of eax is untouched; esi is restored, eax and the flags are the dispatcher's.
// FUNCTION: GP4 0x00510f05
void __cdecl FUN_00510f05(gp4::Regs* r) {
    const uint32_t savedEsi = r->esi;
    GP4_GLOBAL(uint8_t, 0x007a9792) = 0xffu;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007c595c)));
    r->eax = (r->eax & 0xffff0000u) | 0x48u;
    gp4::call_regs(0x00531f9c, *r);
    r->esi = savedEsi;
}
GP4_IMPL(0x00510f05, FUN_00510f05, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=dw_00532d48")
