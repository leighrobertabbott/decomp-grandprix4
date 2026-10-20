#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: esi = the current record [0x00a69878]; 0x00408216 leaves CF set when the signed
// state byte +0xe4 is in 2..5. CF is stored as 0/1 in 0x00a6987c. popal restores every GPR; the
// flags are those 0x00408216 leaves.
// FUNCTION: GP4 0x0051f8b0
void __cdecl FUN_0051f8b0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::call_regs(0x00408216, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = ((c.eflags & gp4::flags::CF) != 0u) ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f8b0, FUN_0051f8b0, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00a69878")
