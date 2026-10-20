#include <gp4/gp4.h>
#include <gp4/flags.h>

// Returns "true" through the registers (`or al, 0xff; stc; ret`): al = 0xff and CF = 1. The other flags
// are those of the 8-bit `or al, 0xff` (result 0xff: PF and SF set, ZF clear, AF = OF = 0).
// FUNCTION: GP4 0x0051ce53
void __cdecl FUN_0051ce53(gp4::Regs* r) {
    r->eax |= 0xffu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0xffu) | gp4::flags::CF);
}
GP4_IMPL(0x0051ce53, FUN_0051ce53, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
