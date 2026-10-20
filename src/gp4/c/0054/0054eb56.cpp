#include <gp4/gp4.h>
#include <gp4/flags.h>

// push eax; mov eax, -1; call 0x0054eb10; pop eax; ret
// Runs the node-list flag routine 0x0054eb10 with eax = -1 (every visited node gets bit 0x20 set, as
// well as 0x40). eax comes back as at entry; esi is preserved by the callee; the flags are those of the
// callee's final compare.
// FUNCTION: GP4 0x0054eb56
void __cdecl FUN_0054eb56(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 0xffffffffu;
    gp4::call_regs(0x0054eb10, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054eb56, FUN_0054eb56, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=list_0054eaf7+links_0054eaf7+tail_0054eb10")
