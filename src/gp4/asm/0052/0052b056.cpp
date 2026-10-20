#include <gp4/gp4.h>
#include <gp4/flags.h>

// Selector dh (edx bits 8..15): dh == 2 returns eax = [0x0052b077 result for the year in ecx] - 0x151
// (flags of that `sub`); any other value returns the byte at 0x81d36f + (dh - 1) (flags of the `dec al`,
// whose CF stays that of `cmp dh, 2`). ebx is saved and restored.
// FUNCTION: GP4 0x0052b056
void __cdecl FUN_0052b056(gp4::Regs* r) {
    const uint32_t selector = (r->edx >> 8) & 0xffu;
    const uint32_t cmpFlags = gp4::flags::sub8(selector, 2u);

    if (selector == 2u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eflags = gp4::flags::merge(r->eflags, cmpFlags);
        gp4::call_regs(0x0052b077, c);
        r->eax = c.eax - 0x151u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(c.eax, 0x151u));
        return;
    }

    const uint32_t index = (selector - 1u) & 0xffu;
    const uint32_t decFlags = (gp4::flags::sub8(selector, 1u) & ~gp4::flags::CF) | (cmpFlags & gp4::flags::CF);
    r->eax = GP4_ARRAY(const uint8_t, 0x0081d36f)[index];
    r->eflags = gp4::flags::merge(r->eflags, decFlags);
}
GP4_IMPL(0x0052b056, FUN_0052b056, "regs(ecx:u32{=0,1,2,3,4,100,400,1900,1996,2000,2001,2002,2004,2100}, edx:u32{=0x0000,0x0105,0x0200,0x0210,0x0300,0x0400,0x0500,0x0600,0x0700,0x0800,0x0900,0x0a00,0x0b00,0x0c00,0x0d00,0xff00,0x8000}) -> (eax, cf, pf, af, zf, sf, of)")
