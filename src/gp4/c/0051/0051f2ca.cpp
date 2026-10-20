#include <gp4/gp4.h>
#include <gp4/flags.h>

// `pushal ; call 0x0050cfc2 ; popal ; ret`: runs the table-lookup routine 0x0050cfc2 (eax = output
// buffer of eight dwords, edi = the lookup table block). popal restores every general register to its
// entry value, so only the flags of the callee's last flag writer remain.
// FUNCTION: GP4 0x0051f2ca
void __cdecl FUN_0051f2ca(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0050cfc2, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f2ca, FUN_0051f2ca, "regs(eax:ptr[0x40]:layout=pos_0050cfc2, edi:rec[0x8943a0,0x160,0..30]) -> (cf, pf, af, zf, sf, of); globals=layout=shade_0050cfc2")
