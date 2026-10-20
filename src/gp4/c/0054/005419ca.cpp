#include <gp4/gp4.h>

// `mov bh, 0 ; mov edx, 2 ; call 0x0054db64 ; ret`: writes the unsigned decimal digits of eax at the
// string cursor edi in the game's digit encoding (bh = 0: no sign, no padding; edx = 2: at least
// three digits with the decimal point after the first). 0x0054db64 keeps every register but edi and
// ends on the flags of its last callee, so the whole register block comes back from it.
// FUNCTION: GP4 0x005419ca
void __cdecl FUN_005419ca(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ebx = r->ebx & 0xffff00ffu;
    c.edx = 2u;
    gp4::call_regs(0x0054db64, c);
    *r = c;
}
GP4_IMPL(0x005419ca, FUN_005419ca, "regs(eax:u32, ebx:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
