#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; call 0x0051e467; popal; ret
// The callee gets the entry registers; popal then restores every general register to its entry
// value but leaves the flags of the callee's last flag writer.
// FUNCTION: GP4 0x0051f1ee
void __cdecl FUN_0051f1ee(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0051e467, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f1ee, FUN_0051f1ee, "regs(ebx:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_0041104f+cf_0041a2b2+fx_005069d0")
