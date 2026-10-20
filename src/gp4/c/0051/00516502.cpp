#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: [0x00a6987c] = 1, then esi = the selected car record [0x00679d30] and
// 0x0044334d runs (its CF reports a recorded due time: `jb` keeps the 1); with CF clear
// [0x00a6987c] becomes 0. Every GPR is restored; the flags are those 0x0044334d leaves.
// FUNCTION: GP4 0x00516502
void __cdecl FUN_00516502(gp4::Regs* r) {
    gp4::Regs c = *r;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = 1u;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x0044334d, c);
    if ((c.eflags & static_cast<uint32_t>(gp4::flags::CF)) == 0u) {
        GP4_GLOBAL(uint32_t, 0x00a6987c) = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00516502, FUN_00516502, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00418f8f+state_0044334d")
