#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper: asks 0x004081a1 about the selected car [0x00679d30] (it answers in CF) and
// stores 1 (CF set) or 0 into [0x00a6987c]. popal restores every register; the flags are those
// 0x004081a1 leaves.
// FUNCTION: GP4 0x0051622c
void __cdecl FUN_0051622c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = 0u;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x004081a1, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = (c.eflags & gp4::flags::CF) != 0u ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051622c, FUN_0051622c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30")
