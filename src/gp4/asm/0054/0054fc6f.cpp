#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dispatcher over the 8-entry handler table at 0x0054fc84, indexed by the low three bits of al.
// pushal; ecx = table + (al & 7) * 4; call [ecx]; popal. The handler sees every register as the caller
// left it except ecx (the table slot address) and the flags of `add ecx, 0x54fc84`; popal restores the
// registers, so only the flags the handler leaves are visible to the caller.
// FUNCTION: GP4 0x0054fc6f
void __cdecl FUN_0054fc6f(gp4::Regs* r) {
    const uint32_t index = r->eax & 7u;
    const uint32_t slot = index << 2;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = slot + 0x0054fc84u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(slot, 0x0054fc84u));
    gp4::call_regs(GP4_ARRAY(uint32_t, 0x0054fc84)[index], c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054fc6f, FUN_0054fc6f, "regs(eax:u32{=4,6,7,0x0c,0xfe,0xff,0x134}, esi:u32{=0x0054fd8c,0x0054fd98,0x0054e232,0x00527d03,0x00531741}) -> (cf, pf, af, zf, sf, of)")
