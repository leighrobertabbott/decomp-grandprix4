#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the token at esi into the 20-byte language name buffer 0x008339b4 ("english") with the token
// copier 0x0054c500 (ecx = 0x14 maximum). ecx is 0x14, esi/edi come back advanced by the copier; the
// flags are the copier's.
// FUNCTION: GP4 0x0054c5b7
void __cdecl FUN_0054c5b7(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = 0x14u;
    c.edi = 0x008339b4u;
    gp4::call_regs(0x0054c500, c);
    r->ecx = 0x14u;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054c5b7, FUN_0054c5b7, "regs(esi:ptr[0x40]:u8[0x20..0x30]) -> (ecx, esi, edi, cf, pf, af, zf, sf, of)")
