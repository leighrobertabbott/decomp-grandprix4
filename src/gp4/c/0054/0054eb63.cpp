#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the list-walk 0x0054eb10 with eax = 0 (every node of the 0x00838c94 list gets bit 5 cleared and
// bit 6 set). eax is pushed and popped, so only the flags the callee leaves are visible.
// FUNCTION: GP4 0x0054eb63
void __cdecl FUN_0054eb63(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 0u;
    gp4::call_regs(0x0054eb10, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054eb63, FUN_0054eb63, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=list_0054eaf7+links_0054eaf7+tail_0054eb10")
