#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dispatcher over the 8-entry handler table at 0x0054fd62, indexed by the low three bits of al (twin of
// 0x0054fc6f). pushal; ecx = table + (al & 7) * 4; call [ecx]; popal. The handler sees every register as
// the caller left it except ecx (the table slot address) and the flags of `add ecx, 0x54fd62`; popal
// restores the registers, so only the flags the handler leaves are visible to the caller.
// FUNCTION: GP4 0x0054fd4d
void __cdecl FUN_0054fd4d(gp4::Regs* r) {
    const uint32_t index = r->eax & 7u;
    const uint32_t slot = index << 2;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = slot + 0x0054fd62u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(slot, 0x0054fd62u));
    gp4::call_regs(GP4_ARRAY(uint32_t, 0x0054fd62)[index], c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054fd4d, FUN_0054fd4d, "regs(eax:u32{=0,1,2,3,4,5,6,7,8,0x09,0x0a,0x0d,0xfb,0xfc,0xfd,0xfe,0xff,0x100}) -> (cf, pf, af, zf, sf, of); globals=layout=case_0054fd4d")
