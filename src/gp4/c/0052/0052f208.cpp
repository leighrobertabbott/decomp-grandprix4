#include <gp4/gp4.h>
#include <gp4/flags.h>

// Translates al through the character table (0x0054d755), then classifies it: 'a'..'z' gives al - 0x61
// and '0'..'5' gives al - 0x16 (CF clear, other flags those of the sub); anything else leaves al as
// translated and sets CF (other flags those of the last compare). clc/stc replace CF only.
// FUNCTION: GP4 0x0052f208
void __cdecl FUN_0052f208(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0054d755, c);
    const uint32_t al = c.eax & 0xffu;
    uint32_t out = al;
    uint32_t flags;
    bool bad = false;
    if (al < 0x61u) {
        flags = gp4::flags::sub8(al, 0x30u);
        if (al < 0x30u) {
            bad = true;
        } else {
            flags = gp4::flags::sub8(al, 0x35u);
            if (al > 0x35u) {
                bad = true;
            } else {
                out = (al - 0x16u) & 0xffu;
                flags = gp4::flags::sub8(al, 0x16u);
            }
        }
    } else {
        flags = gp4::flags::sub8(al, 0x7au);
        if (al > 0x7au) {
            bad = true;
        } else {
            out = (al - 0x61u) & 0xffu;
            flags = gp4::flags::sub8(al, 0x61u);
        }
    }
    flags = (flags & ~1u) | (bad ? 1u : 0u);
    r->eax = (c.eax & 0xffffff00u) | out;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052f208, FUN_0052f208, "regs(eax:u32{0x30,0x31,0x33,0x35,0x2f,0x36,0x41,0x46,0x61,0x62,0x7a,0x7b,0x60,0x20,0x5a}) -> (eax, cf, pf, af, zf, sf, of); globals=layout=lower_00833e18")
