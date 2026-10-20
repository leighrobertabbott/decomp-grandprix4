#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper around the stdcall text-capture routine 0x004fd730: pushes [0x00833cc9] (text row),
// [0x00833cc5] (text column) and eax (the character in al), calls it and pops every register back.
// popal does not restore the flags, so those of the callee remain.
// FUNCTION: GP4 0x0054d3a1
void __cdecl FUN_0054d3a1(gp4::Regs* r) {
    const uint32_t args[3] = {r->eax, GP4_GLOBAL(uint32_t, 0x00833cc5), GP4_GLOBAL(uint32_t, 0x00833cc9)};
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs_stdcall(0x004fd730, c, args);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054d3a1, FUN_0054d3a1, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=log_004fd730")
