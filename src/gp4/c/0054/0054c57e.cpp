#include <gp4/gp4.h>
#include <gp4/flags.h>

// Keyword lookup wrapper: runs the keyword matcher 0x0054c4c3 on the token at esi (carry clear = found,
// esi advanced past the token, al = the value) and, only when it matched, stores that byte to 0x008339ac.
// eax, esi and the flags are the matcher's; the other registers come back unchanged.
// FUNCTION: GP4 0x0054c57e
void __cdecl FUN_0054c57e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054c4c3, c);
    if ((c.eflags & gp4::flags::CF) == 0u) {
        GP4_GLOBAL(uint8_t, 0x008339ac) = static_cast<uint8_t>(c.eax);
    }
    r->eax = c.eax;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054c57e, FUN_0054c57e, "regs(esi:ptr[0x40]:layout=kw_0054c4c3) -> (eax, esi, cf, pf, af, zf, sf, of)")
