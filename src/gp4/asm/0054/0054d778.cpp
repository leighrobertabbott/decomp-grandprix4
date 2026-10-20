#include <gp4/gp4.h>
#include <gp4/flags.h>

// esi = (eax % 100) + 0x6e: the unsigned remainder of eax by 100 (the `div` of edx:eax, edx = 0) plus a
// base of 0x6e. eax, ebx and edx are saved and restored; the flags are those of `add esi, 0x6e`.
// FUNCTION: GP4 0x0054d778
void __cdecl FUN_0054d778(gp4::Regs* r) {
    const uint32_t remainder = r->eax % 100u;
    r->esi = remainder + 0x6eu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(remainder, 0x6eu));
}
GP4_IMPL(0x0054d778, FUN_0054d778, "regs(eax:u32) -> (esi, cf, pf, af, zf, sf, of)")
