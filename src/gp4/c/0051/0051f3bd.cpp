#include <gp4/gp4.h>

// Tail wrapper: `call 0x0051d2ec ; ret`. al is the character; the callee either stores it at [edi]
// (stosb, when [0x89152b] == 1) or draws the glyph inside a pushal/popal pair. Registers and flags
// come straight back from it.
// FUNCTION: GP4 0x0051f3bd
void __cdecl FUN_0051f3bd(gp4::Regs* r) {
    gp4::call_regs(0x0051d2ec, *r);
}
GP4_IMPL(0x0051f3bd, FUN_0051f3bd, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
