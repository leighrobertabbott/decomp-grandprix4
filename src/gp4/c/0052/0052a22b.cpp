#include <gp4/gp4.h>
#include <gp4/flags.h>

// Mirror of 0x0052a202: the saved byte 0x0081d228 is put into 0x0081d1f8 (the 7th byte of the ramp
// at 0x0081d1f2) and 0x00410c46 expands the 7-byte ramp (eax = edx = 0x0081d1f2); then the old byte
// 0x0081d1f8 comes back. pushal/popal restore every register; the flags are those 0x00410c46 leaves.
// FUNCTION: GP4 0x0052a22b
void __cdecl FUN_0052a22b(gp4::Regs* r) {
    const uint8_t saved = GP4_GLOBAL(uint8_t, 0x0081d1f8);
    const uint8_t stored = GP4_GLOBAL(uint8_t, 0x0081d228);
    GP4_GLOBAL(uint8_t, 0x0081d1f8) = stored;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = (c.ebx & 0xffffff00u) | saved;
    c.ecx = (c.ecx & 0xffffff00u) | stored;
    c.eax = 0x0081d1f2u;
    c.edx = 0x0081d1f2u;
    gp4::call_regs(0x00410c46, c);

    GP4_GLOBAL(uint8_t, 0x0081d1f8) = saved;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052a22b, FUN_0052a22b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ramp_0081d1f2")
