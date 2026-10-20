#include <gp4/gp4.h>

// "push esi; mov esi, 0x7c595c; mov ax, 0x46; call 0x00531f9c; pop esi; ret": dispatcher command 0x46
// with esi pointing at the table 0x007c595c. The upper half of eax is untouched; esi is restored, eax
// and the flags are the dispatcher's.
// FUNCTION: GP4 0x00510eea
void __cdecl FUN_00510eea(gp4::Regs* r) {
    const uint32_t savedEsi = r->esi;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007c595c)));
    r->eax = (r->eax & 0xffff0000u) | 0x46u;
    gp4::call_regs(0x00531f9c, *r);
    r->esi = savedEsi;
}
GP4_IMPL(0x00510eea, FUN_00510eea, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
