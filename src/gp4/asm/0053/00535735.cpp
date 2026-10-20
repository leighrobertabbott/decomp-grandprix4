#include <gp4/gp4.h>
#include <gp4/flags.h>

// Formats eax + 0x13 (`sub eax, 0x53 ; add eax, 0x66`) as a decimal number through 0x0054db64
// with edx = 2 (at least three digits, a decimal point after the first) and bh = 0 (no sign, no
// padding): the number is written at the string cursor edi. 0x0054db64 keeps every register but edi
// and ends on the flags of its last callee, so the whole register block comes back from it.
// FUNCTION: GP4 0x00535735
void __cdecl FUN_00535735(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = r->eax - 0x53u + 0x66u;
    c.ebx = r->ebx & 0xffff00ffu;
    c.edx = 2u;
    gp4::call_regs(0x0054db64, c);
    *r = c;
}
GP4_IMPL(0x00535735, FUN_00535735, "regs(eax:u32, ebx:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
