#include <gp4/gp4.h>

// pushal wrapper: runs 0x0040ed93 with esi = the selected car record [0x00679d30] and stores its
// result eax in 0x00a6987c. popal restores every GPR; the flags stay those 0x0040ed93 leaves.
// FUNCTION: GP4 0x0051682d
void __cdecl FUN_0051682d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x0040ed93, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = c.eax;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0051682d, FUN_0051682d, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_0044334d+time_0040ed93")
