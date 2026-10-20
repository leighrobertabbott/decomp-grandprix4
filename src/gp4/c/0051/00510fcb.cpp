#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: command 0x86 of the dispatcher 0x00531f9c stores the performance counter in the
// qword 0x007a8e04; its low dword then selects how many steps 0x004080a6 advances the generator
// (eax & 0xfff, at least 10), and bit 0 of the counter is saved in 0x007a8e10. popal restores every
// register but not the flags, which are those of the final `and eax, 1`.
// FUNCTION: GP4 0x00510fcb
void __cdecl FUN_00510fcb(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = (c.eax & 0xffff0000u) | 0x86u;
    gp4::call_regs(0x00531f9c, c);
    c.eax = GP4_GLOBAL(uint32_t, 0x007a8e04);
    gp4::call_regs(0x004080a6, c);
    const uint32_t bit = GP4_GLOBAL(uint32_t, 0x007a8e04) & 1u;
    GP4_GLOBAL(uint32_t, 0x007a8e10) = bit;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(bit));
}
GP4_IMPL(0x00510fcb, FUN_00510fcb, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=qpc_0054e733+state_0066d8c0")
