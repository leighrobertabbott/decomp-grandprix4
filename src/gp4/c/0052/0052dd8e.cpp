#include <gp4/gp4.h>

// pushal wrapper: runs the key-combination formatter 0x0052dda1 with eax = key code [0x00a69878] and
// edi = output cursor [0x00a6987c]. popal restores every GPR (the advanced edi is discarded); the
// flags stay those 0x0052dda1 leaves.
// FUNCTION: GP4 0x0052dd8e
void __cdecl FUN_0052dd8e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x00a69878);
    c.edi = GP4_GLOBAL(uint32_t, 0x00a6987c);
    gp4::call_regs(0x0052dda1, c);
    r->eflags = c.eflags;
}
GP4_IMPL(0x0052dd8e, FUN_0052dd8e, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
