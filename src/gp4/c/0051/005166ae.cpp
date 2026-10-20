#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: stores the sort key 0x00516daa returns for list index 0 / table selector 1 to
// 0x00a6987c. popal restores every GPR; the flags are those 0x00516daa leaves (`cmp edx, 1`).
// FUNCTION: GP4 0x005166ae
void __cdecl FUN_005166ae(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.edx = 1u;
    c.eax = 0u;
    gp4::call_regs(0x00516daa, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005166ae, FUN_005166ae, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=order_00675958+keys_006759c8")
