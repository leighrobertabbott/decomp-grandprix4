#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the 0x005202a4 pass only when the mode byte [0x007c2ea9] is 0x80 and bit 6 of the flag byte [0x007c2ead] is
// set (mode 0x44 only tests bit 5 of the flag byte and does nothing). The flags are those of the last compare or
// test executed, or the callee's when it ran.
// FUNCTION: GP4 0x0052046a
void __cdecl FUN_0052046a(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t arith = gp4::flags::sub8(mode, 0x44u);
    if (mode == 0x44u) {
        arith = gp4::flags::logic8(static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x007c2ead) & 0x20u));
    } else {
        arith = gp4::flags::sub8(mode, 0x80u);
        if (mode == 0x80u) {
            const uint8_t bit6 = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x007c2ead) & 0x40u);
            arith = gp4::flags::logic8(bit6);
            if (bit6 != 0u) {
                gp4::Regs c = *r;
                c.st_in = 0;
                c.st_out = 0;
                c.st_xin_mask = 0;
                c.st_xout_mask = 0;
                gp4::call_regs(0x005202a4, c);
                r->eax = c.eax;
                r->edx = c.edx;
                arith = c.eflags;
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052046a, FUN_0052046a, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+state_0040e84e+carno_0066ff6c+cars_0066fef0+lap_0043b482+state_0044334d+state_0040dfe9+pick_00443148+rank_005202a4")
