#include <gp4/gp4.h>
#include <gp4/flags.h>

// `pushal ; call 0x0051488e ; popal ; ret`: runs the HUD/option text parser (it reads the text buffer at
// 0x00891ce0 and fills the record table 0x00891bd0). popal restores every general register to its entry
// value, so only the flags of the callee's last flag writer remain.
// FUNCTION: GP4 0x005146e0
void __cdecl FUN_005146e0(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0051488e, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005146e0, FUN_005146e0, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=text_00891ce0")
