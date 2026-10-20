#include <gp4/gp4.h>

// pushal wrapper: loads ecx, edx and eax from 0x00a69878, 0x00a6987c and 0x00a69880 and calls 0x00444899
// (which stores them in 0x0062a04c / 0x0062a048 / 0x0062a044). popal restores every GPR, so only the
// flags of the callee's final `cmp ecx, 0x400` remain.
// FUNCTION: GP4 0x00551a4f
void __cdecl FUN_00551a4f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = GP4_GLOBAL(uint32_t, 0x00a69878);
    c.edx = GP4_GLOBAL(uint32_t, 0x00a6987c);
    c.eax = GP4_GLOBAL(uint32_t, 0x00a69880);
    gp4::call_regs(0x00444899, c);
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00551a4f, FUN_00551a4f, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
