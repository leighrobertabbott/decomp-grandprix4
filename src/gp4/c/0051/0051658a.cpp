#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: clears 0x00a6987c, then asks 0x00411017 (esi = selected car record [0x00679d30])
// for its result and stores eax there. popal restores every register but not the flags, so the
// flags are those the callee leaves.
// FUNCTION: GP4 0x0051658a
void __cdecl FUN_0051658a(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00a6987c) = 0u;
    gp4::Regs c = *r;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x00411017, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051658a, FUN_0051658a, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_0044334d+state_00411017")
