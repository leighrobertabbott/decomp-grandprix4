#include <gp4/gp4.h>

// pushal wrapper: runs 0x00418901 with esi = the first selected car [0x0066d820] and stores its
// result eax (address of the 0xa8-byte entry chosen by the byte at record + 0x202) in 0x00a6987c.
// popal restores every GPR; the flags stay those 0x00418901 leaves.
// FUNCTION: GP4 0x0051adee
void __cdecl FUN_0051adee(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x0066d820);
    gp4::call_regs(0x00418901, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = c.eax;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0051adee, FUN_0051adee, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_00437888+selcar_0051adee")
