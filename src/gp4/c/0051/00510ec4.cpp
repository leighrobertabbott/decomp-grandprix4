#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the empty routine 0x0051d15f, stores 0xff to the byte 0x00934e90 and ends with
// `cmp byte [0x7ad869], 0; je <next instruction>`: both branch directions reach the same ret, so
// the byte read only shows in the returned flags (those of the cmp).
// FUNCTION: GP4 0x00510ec4
void __cdecl FUN_00510ec4(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0051d15f, c);
    GP4_GLOBAL(uint8_t, 0x00934e90) = 0xffu;
    const uint8_t flag = GP4_GLOBAL(uint8_t, 0x007ad869);
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(flag, 0u));
}
GP4_IMPL(0x00510ec4, FUN_00510ec4, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
