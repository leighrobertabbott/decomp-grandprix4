#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: saves the current state into slot 1 (0x00510309), then restores slot 2
// (0x0051006c). popal restores every GPR; the flags are whatever the restore routine leaves.
// FUNCTION: GP4 0x00510596
void __cdecl FUN_00510596(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 1u;
    gp4::call_regs(0x00510309, c);
    c.eax = 2u;
    gp4::call_regs(0x0051006c, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00510596, FUN_00510596, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=save_00510309")
