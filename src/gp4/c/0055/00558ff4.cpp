#include <gp4/gp4.h>
#include <gp4/flags.h>

// Under pushal/popal: tests the flag word 0x0096548c against 0x7f800 and, when none of those bits is set, runs
// the ring-record bit clearing pass 0x00558f50. Every register is kept; the flags are those of the test or of
// the callee's last compare.
// FUNCTION: GP4 0x00558ff4
void __cdecl FUN_00558ff4(gp4::Regs* r) {
    const uint32_t bits = GP4_GLOBAL(uint32_t, 0x0096548c) & 0x7f800u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(bits));
    if (bits == 0u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x00558f50, c);
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
    }
}
GP4_IMPL(0x00558ff4, FUN_00558ff4, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_0051cc50+attr_00558e7b+ring_00558f50")
