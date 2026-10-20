#include <gp4/gp4.h>
#include <gp4/flags.h>
#include <gp4/x87.h>

// Start-up: pushal; 0x00527453 (fninit and the control words); then the angle scales are derived from
// the fixed-point unit constants, every quotient computed on the x87 stack (80-bit pi / constant, or
// constant / pi) and stored as a qword: 0x6f2be4 = [0x6f2b64] / pi and the four unit scales
// 0x6f2bbc / 0x6f2bc4 / 0x6f2bcc / 0x6f2bd4 = pi / [0x6f2b34], [0x6f2b44], [0x6f2b4c], [0x6f2b5c] and
// 0x6f2bdc = pi / [0x6f2b64]. popal restores every GPR; the flags are those 0x00527453 leaves.
// FUNCTION: GP4 0x00527476
void __cdecl FUN_00527476(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00527453, c);

    GP4_GLOBAL(double, 0x006f2be4) = gp4::x87::narrow(gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b64)) / gp4::x87::ext_pi());
    GP4_GLOBAL(double, 0x006f2bbc) = gp4::x87::narrow(gp4::x87::ext_pi() / gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b34)));
    GP4_GLOBAL(double, 0x006f2bc4) = gp4::x87::narrow(gp4::x87::ext_pi() / gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b44)));
    GP4_GLOBAL(double, 0x006f2bcc) = gp4::x87::narrow(gp4::x87::ext_pi() / gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b4c)));
    GP4_GLOBAL(double, 0x006f2bd4) = gp4::x87::narrow(gp4::x87::ext_pi() / gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b5c)));
    GP4_GLOBAL(double, 0x006f2bdc) = gp4::x87::narrow(gp4::x87::ext_pi() / gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b64)));

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00527476, FUN_00527476, "regs() -> (cf, pf, af, zf, sf, of)")
