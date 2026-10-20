#include <gp4/gp4.h>

// pushal wrapper that finishes the debris-part bookkeeping: runs the bounding pass 0x00422642, the
// averaging pass 0x00444a71, the squaring pass 0x00444a84 and the pre-multiplying pass 0x00444ab0 in turn.
// popal restores every register; the flags are those 0x00444ab0 leaves (its final `add edi, 0x10`).
// FUNCTION: GP4 0x004448b3
void __cdecl FUN_004448b3(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x00422642, c);
    gp4::call_regs(0x00444a71, c);
    gp4::call_regs(0x00444a84, c);
    gp4::call_regs(0x00444ab0, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x004448b3, FUN_004448b3, "regs(ebp:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_0042458b+state_0042257b+state_004225c0+list_0062a048+count_0062a04c+parts_0062a5ac; wall=2400")
