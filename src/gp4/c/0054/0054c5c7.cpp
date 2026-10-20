#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the token at esi into the 31-byte buffer 0x008339c8 with the token copier 0x0054c500
// (ecx = 0x1f maximum). ecx is 0x1f, esi/edi come back advanced by the copier; the flags are the
// copier's.
// FUNCTION: GP4 0x0054c5c7
void __cdecl FUN_0054c5c7(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = 0x1fu;
    c.edi = 0x008339c8u;
    gp4::call_regs(0x0054c500, c);
    r->ecx = 0x1fu;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054c5c7, FUN_0054c5c7, "regs(esi:ptr[0x40]:u8[0x20..0x30]) -> (ecx, esi, edi, cf, pf, af, zf, sf, of)")
