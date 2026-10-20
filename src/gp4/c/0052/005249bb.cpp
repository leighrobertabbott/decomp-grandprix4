#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs 0x00524f0f only while the byte [0x00852d94] is nonzero. The flags are those of
// `test byte [0x852d94], 0xff` when it is zero, else those 0x00524f0f leaves; every register is kept.
// FUNCTION: GP4 0x005249bb
void __cdecl FUN_005249bb(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint8_t, 0x00852d94);
    const uint32_t arith = gp4::flags::logic8(value);
    if (value == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x00524f0f, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005249bb, FUN_005249bb, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040f5f3+order_00418f8f+gate_00524f0f")
