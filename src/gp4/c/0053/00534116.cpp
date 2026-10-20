#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the list-head setup 0x00534123 twice: once with the registers as given, then again with eax and
// esi each raised by one (`inc` keeps CF, so the flags it hands to the second run are those of the
// first run with the inc flags merged in). eax and esi come back raised by one; the flags are the
// second run's.
// FUNCTION: GP4 0x00534116
void __cdecl FUN_00534116(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x00534123, c);

    uint32_t flags = c.eflags;
    flags = gp4::flags::merge(flags, gp4::flags::inc32(c.eax, flags));
    c.eax += 1u;
    flags = gp4::flags::merge(flags, gp4::flags::inc32(c.esi, flags));
    c.esi += 1u;
    c.eflags = flags;
    gp4::call_regs(0x00534123, c);

    r->eax = c.eax;
    r->esi = c.esi;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00534116, FUN_00534116, "regs(eax:u32[0..14], esi:u32[0..0x3fe]) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=head_00852837+tab_00835846")
