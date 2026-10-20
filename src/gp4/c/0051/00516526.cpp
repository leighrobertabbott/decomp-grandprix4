#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: [0x00a6987c] = 0, then, when the mode byte [0x007c2ea9] is 0, esi = the selected car record
// [0x00679d30] and 0x004081cc runs; with its CF clear (`jb` not taken) [0x00a6987c] becomes 1. Every GPR is
// restored; the flags are those of `cmp byte [0x007c2ea9], 0`, or the ones 0x004081cc leaves.
// FUNCTION: GP4 0x00516526
void __cdecl FUN_00516526(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = 0u;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t arith = gp4::flags::sub8(mode, 0u);
    if (mode == 0u) {
        gp4::call_regs(0x004081cc, c);
        arith = c.eflags;
        if ((c.eflags & gp4::CF) == 0u) {
            GP4_GLOBAL(uint32_t, 0x00a6987c) = 1u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00516526, FUN_00516526, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30+carno_0066ff6c")
