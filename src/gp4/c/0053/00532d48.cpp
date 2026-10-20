#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte 0x007ad869 is zero, runs 0x00541eba with eax = 0x100 (it stores esi to 0x007a9788,
// clears 0x007a9790 and copies 0x007a9784 to 0x007a9780, leaving eax = that value). Otherwise only
// the flags of `cmp byte [0x7ad869], 0` are left.
// FUNCTION: GP4 0x00532d48
void __cdecl FUN_00532d48(gp4::Regs* r) {
    const uint8_t flag = GP4_GLOBAL(uint8_t, 0x007ad869);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(flag, 0u));
    if (flag == 0u) {
        gp4::Regs c = *r;
        c.eax = 0x100u;
        gp4::call_regs(0x00541eba, c);
        r->eax = c.eax;
        r->eflags = c.eflags;
    }
}
GP4_IMPL(0x00532d48, FUN_00532d48, "regs(esi:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=dw_00532d48")
