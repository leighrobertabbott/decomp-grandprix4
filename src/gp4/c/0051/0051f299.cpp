#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; call 0x0050c980; popal; ret
// The callee gets the entry registers (edi = a 0x160-byte track ring record); popal then restores
// every general register to its entry value but leaves the flags of the callee's last flag writer.
// FUNCTION: GP4 0x0051f299
void __cdecl FUN_0051f299(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0050c980, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f299, FUN_0051f299, "regs(edi:rec[0x8943a0,0x160,0..25]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_00555b28+marks_0050c980")
