#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks the token at esi up in the keyword table at 0x00833a2c with the matcher 0x0054c4d3 (carry clear =
// found, esi advanced past the token, eax = the matching table entry). On a match eax becomes the dword
// at entry +4 (the handler / value); on a miss (carry set) eax stays the entry pointer the matcher left.
// The matcher keeps ebx/ecx/edi; the flags are the matcher's.
// FUNCTION: GP4 0x0054c4c3
void __cdecl FUN_0054c4c3(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0x00833a2cu;
    gp4::call_regs(0x0054c4d3, c);
    uint32_t eax = c.eax;
    if ((c.eflags & gp4::flags::CF) == 0u) {
        eax = GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.eax)), 4);
    }
    r->eax = eax;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054c4c3, FUN_0054c4c3, "regs(esi:ptr[0x40]:layout=kw_0054c4c3) -> (eax, esi, cf, pf, af, zf, sf, of)")
