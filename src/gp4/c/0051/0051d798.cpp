#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper that copies the two control blocks into the selected car records. When a car record
// is selected ([0x0066d820] != 0), the control block 0x007ab548 is applied to it by 0x004112dd and
// its six words are mirrored into the display bytes by 0x0051d4b5; when [0x007aafd4] is nonzero and a
// second record is selected ([0x0066d828] != 0), the block 0x007ab56c is applied to that one. Then
// [0x0095404e] = 0xff and 0x0040fa33 runs. popal restores every register; the flags are those left
// by 0x0040fa33.
// FUNCTION: GP4 0x0051d798
void __cdecl FUN_0051d798(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.esi = GP4_GLOBAL(uint32_t, 0x0066d820);
    if (c.esi != 0u) {
        c.ebx = 0x007ab548u;
        gp4::call_regs(0x004112dd, c);
        c.ebx = 0x007ab548u;
        gp4::call_regs(0x0051d4b5, c);
    }
    if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u) {
        c.esi = GP4_GLOBAL(uint32_t, 0x0066d828);
        if (c.esi != 0u) {
            c.ebx = 0x007ab56cu;
            gp4::call_regs(0x004112dd, c);
        }
    }
    GP4_GLOBAL(uint8_t, 0x0095404e) = 0xffu;
    gp4::call_regs(0x0040fa33, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051d798, FUN_0051d798, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=flag_007a97a0+ctl_007ab548+ctl_007ab56c+cars_0040fa51+state_0040fa01+carcf_0051d798+ctlw_007ab55a")
