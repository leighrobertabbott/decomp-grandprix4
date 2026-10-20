#include <gp4/gp4.h>
#include <gp4/flags.h>

// Probe of the esi record's selected dword (0x00516ddf, list index 0, selector [0x0089151c]): the
// carry flag reports whether it equals 0x20000000 (`cmp eax, 0x20000000` / `stc` when equal, else
// `clc`). eax = the dword, edx = the selector; the other flags are those of the compare.
// FUNCTION: GP4 0x00516e0f
void __cdecl FUN_00516e0f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0u;
    c.edx = GP4_GLOBAL(uint32_t, 0x0089151c);
    gp4::call_regs(0x00516ddf, c);

    const uint32_t carry = (c.eax == 0x20000000u) ? gp4::flags::CF : 0u;
    const uint32_t flags = (gp4::flags::sub32(c.eax, 0x20000000u) & ~gp4::flags::CF) | carry;
    r->eax = c.eax;
    r->edx = c.edx;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00516e0f, FUN_00516e0f, "regs(esi:ptr[0x30]:u32{0x20000000,0x20000000,1,0x7fffffff}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
