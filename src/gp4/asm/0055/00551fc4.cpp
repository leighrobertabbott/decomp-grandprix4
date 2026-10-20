#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags-only test for esi == 0xa6: leaves the flags of `cmp esi, 0xa6`, except that when esi is 0xa6
// the following `or eax, eax` replaces them with the logic flags of eax. Registers are unchanged.
// FUNCTION: GP4 0x00551fc4
void __cdecl FUN_00551fc4(gp4::Regs* r) {
    uint32_t flags = gp4::flags::sub32(r->esi, 0xa6u);
    if (r->esi == 0xa6u) {
        flags = gp4::flags::logic32(r->eax);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00551fc4, FUN_00551fc4, "regs(eax:i32[-2..2], esi:u32[0xa0..0xac]) -> (cf, pf, af, zf, sf, of)")
