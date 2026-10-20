#include <gp4/gp4.h>
#include <gp4/flags.h>

// Parses an unsigned decimal number at esi with 0x0054c495 and, when at least one digit was read
// (carry clear), stores it in the dword 0x008339b0. eax, esi and the flags come back as the parser
// left them (the store touches no flag).
// FUNCTION: GP4 0x0054c5aa
void __cdecl FUN_0054c5aa(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054c495, c);
    if ((c.eflags & static_cast<uint32_t>(gp4::flags::CF)) == 0u) {
        GP4_GLOBAL(uint32_t, 0x008339b0) = c.eax;
    }
    r->eax = c.eax;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054c5aa, FUN_0054c5aa, "regs(esi:ptr[0x40]:u8{0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x39,0x30,0x35,0x20,0x2f,0x3a,0x00,0x41}) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
