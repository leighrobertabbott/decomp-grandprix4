#include <gp4/gp4.h>

// pushal; call 0x004448b3; popal; ret
// The callee gets the entry registers; popal then restores every general register to its entry
// value but leaves the flags of the callee's last flag writer.
// FUNCTION: GP4 0x00551a68
void __cdecl FUN_00551a68(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x004448b3, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00551a68, FUN_00551a68, "regs(ebp:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_0042458b+state_0042257b+state_004225c0+list_0062a048+count_0062a04c+parts_0062a5ac; wall=2400")
