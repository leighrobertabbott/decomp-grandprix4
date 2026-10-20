#include <gp4/gp4.h>
#include <gp4/flags.h>

// esi = (eax % 100) + 0x45: the unsigned remainder of eax by 100 (the `div` of edx:eax, edx = 0) plus a
// base of 0x45. eax, ebx and edx are saved and restored; the flags are those of `add esi, 0x45`.
// FUNCTION: GP4 0x0054d763
void __cdecl FUN_0054d763(gp4::Regs* r) {
    const uint32_t remainder = r->eax % 100u;
    r->esi = remainder + 0x45u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(remainder, 0x45u));
}
GP4_IMPL(0x0054d763, FUN_0054d763, "regs(eax:u32) -> (esi, cf, pf, af, zf, sf, of)")
