#include <gp4/gp4.h>

// pushal wrapper: runs 0x00433d26 with esi = the record pointer [0x00a69878] and stores its result eax
// in 0x00a6987c. popal restores every GPR; the flags stay those 0x00433d26 leaves.
// FUNCTION: GP4 0x0051f86b
void __cdecl FUN_0051f86b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::call_regs(0x00433d26, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = c.eax;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0051f86b, FUN_0051f86b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00a69878")
