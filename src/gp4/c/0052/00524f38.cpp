#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / popal gate on the HUD refresh on the byte 0x0066d7a4: when it is 0x80 the HUD refresh 0x0052537b runs, otherwise
// nothing does. popal restores every register; the flags are those of the compare (no refresh) or of
// the callee.
// FUNCTION: GP4 0x00524f38
void __cdecl FUN_00524f38(gp4::Regs* r) {
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    const uint32_t arith = gp4::flags::sub8(mode, 0x80u);
    if (mode != 0x80u) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x0052537b, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00524f38, FUN_00524f38, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=gate_00524f0f+ptr_00679d30+gate_00524f4b+ptrtab_00524fa4+ptrs_006f2f6c+ring_00525205+pos_00525073+state_0040f5f3+list_0066fe3c+tabs_00525595+state_0044334d+lap_0043b482+scan_00409aec+state_0062d1b4+objs_00524f38")
