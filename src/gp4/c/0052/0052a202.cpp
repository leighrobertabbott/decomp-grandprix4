#include <gp4/gp4.h>
#include <gp4/flags.h>

// Rebuilds the 7-byte colour ramp at 0x0081d1f2 with 0x00410c1b (source and destination both
// 0x0081d1f2: six bytes clamped, the 7th a sign mask landing in the byte 0x0081d1f8), keeps that 7th
// byte in 0x0081d228 and puts the old byte 0x0081d1f8 back. pushal/popal restore every register; the
// flags are those 0x00410c1b leaves (the byte moves touch none).
// FUNCTION: GP4 0x0052a202
void __cdecl FUN_0052a202(gp4::Regs* r) {
    const uint8_t saved = GP4_GLOBAL(uint8_t, 0x0081d1f8);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = (c.ebx & 0xffffff00u) | saved;
    c.eax = 0x0081d1f2u;
    c.edx = 0x0081d1f2u;
    gp4::call_regs(0x00410c1b, c);

    GP4_GLOBAL(uint8_t, 0x0081d228) = GP4_GLOBAL(uint8_t, 0x0081d1f8);
    GP4_GLOBAL(uint8_t, 0x0081d1f8) = saved;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052a202, FUN_0052a202, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ramp_0081d1f2")
