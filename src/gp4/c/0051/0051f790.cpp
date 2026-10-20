#include <gp4/gp4.h>

// Tail wrapper: `call 0x00510e98 ; ret`. The callee writes the hundreds digit (or a blank) of al
// through 0x0051f3bd and then the remaining two digits through 0x00510e71; registers and flags come
// straight back from it.
// FUNCTION: GP4 0x0051f790
void __cdecl FUN_0051f790(gp4::Regs* r) {
    gp4::call_regs(0x00510e98, *r);
}
GP4_IMPL(0x0051f790, FUN_0051f790, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
