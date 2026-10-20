#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the race clock [0x0066d814] equals [0x00954058]: runs 0x0051d798 (copies the control blocks
// into the selected car records), sets the byte [0x0095404e] to 0xff and runs 0x0040fa33. eax holds
// the clock on return; the flags are those of `cmp eax, [0x00954058]` (clock differs) or those left
// by 0x0040fa33.
// FUNCTION: GP4 0x0051d7e2
void __cdecl FUN_0051d7e2(gp4::Regs* r) {
    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    const uint32_t stamp = GP4_GLOBAL(uint32_t, 0x00954058);
    r->eax = clock;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(clock, stamp));
    if (clock != stamp) {
        return;
    }
    gp4::Regs c = *r;
    gp4::call_regs(0x0051d798, c);
    GP4_GLOBAL(uint8_t, 0x0095404e) = 0xffu;
    gp4::call_regs(0x0040fa33, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051d7e2, FUN_0051d7e2, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=flag_007a97a0+ctl_007ab548+ctl_007ab56c+cars_0040fa51+state_0040fa01+carcf_0051d798+ctlw_007ab55a")
