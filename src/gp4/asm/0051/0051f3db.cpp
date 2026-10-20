#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes the low nibble of al as one hex digit ('0'..'9', 'A'..'F') through 0x0051f3bd.
// eax is restored (push/pop); the other registers and the flags are the callee's.
// FUNCTION: GP4 0x0051f3db
void __cdecl FUN_0051f3db(gp4::Regs* r) {
    const uint32_t value = r->eax;
    const uint32_t nibble = value & 0xfu;
    const uint32_t digit = nibble < 0xau ? nibble + 0x30u : nibble + 0x37u;
    gp4::Regs c = *r;
    c.eax = (value & 0xffffff00u) | digit;
    gp4::call_regs(0x0051f3bd, c);
    c.eax = value;
    *r = c;
}
GP4_IMPL(0x0051f3db, FUN_0051f3db, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
