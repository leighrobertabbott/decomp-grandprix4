#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: asks 0x004080d1 about the selected car record [0x00679d30] (it returns CF = 1 when
// bit 3 of the byte +0xe6 is clear) and stores that carry as 1/0 in 0x00a6987c. popal restores every
// register but not the flags, which are those the callee leaves.
// FUNCTION: GP4 0x00515bd9
void __cdecl FUN_00515bd9(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 0u;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x004080d1, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = (c.eflags & gp4::CF) != 0u ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00515bd9, FUN_00515bd9, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_0044334d")
