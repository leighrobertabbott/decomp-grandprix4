#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: esi = the selected car record [0x00679d30]; [0x00a69878] is cleared, then
// 0x00432271 (test byte [esi+0x26b], 8; stc/clc) runs and [0x00a69878] becomes 1 when it reports CF.
// Every GPR is restored; the flags are those 0x00432271 leaves.
// FUNCTION: GP4 0x0051646c
void __cdecl FUN_0051646c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    GP4_GLOBAL(uint32_t, 0x00a69878) = 0u;
    gp4::call_regs(0x00432271, c);
    if ((c.eflags & static_cast<uint32_t>(gp4::flags::CF)) != 0u) {
        GP4_GLOBAL(uint32_t, 0x00a69878) = 1u;
    }
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051646c, FUN_0051646c, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ptr_00679d30")
