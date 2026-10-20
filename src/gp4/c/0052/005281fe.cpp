#include <gp4/gp4.h>
#include <gp4/flags.h>

// Animation tick: when the clock [0x007a9478] is more than one tick (signed) past the last pass
// [0x0081ce71], the last pass is brought up to date and the animation slots are closed (0x0052822a with
// al = 0: only the slots that advanced) and stepped (0x005282fb). eax is pushed and restored and both
// callees are pushal/popal wrappers; the flags are those of `cmp eax, 1` when nothing runs, else those
// the second callee leaves.
// FUNCTION: GP4 0x005281fe
void __cdecl FUN_005281fe(gp4::Regs* r) {
    const int32_t elapsed = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x007a9478) - GP4_GLOBAL(uint32_t, 0x0081ce71));
    uint32_t f = gp4::flags::sub32(static_cast<uint32_t>(elapsed), 1u);
    if (elapsed > 1) {
        GP4_GLOBAL(uint32_t, 0x0081ce71) = GP4_GLOBAL(uint32_t, 0x007a9478);
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = 0u;
        gp4::call_regs(0x0052822a, c);
        gp4::call_regs(0x005282fb, c);
        f = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x005281fe, FUN_005281fe, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=anim_005282fb+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+list_00549db9+pool_00549db9")
