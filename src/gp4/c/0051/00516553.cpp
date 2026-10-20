#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: [0x00a6987c] = 0, then esi = the selected car record [0x00679d30] and
// 0x004081cc runs (CF = bit 0x40 of the car byte +0x162); with CF clear [0x00a6987c] becomes 1.
// Every GPR is restored; the flags are those 0x004081cc leaves.
// FUNCTION: GP4 0x00516553
void __cdecl FUN_00516553(gp4::Regs* r) {
    gp4::Regs c = *r;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = 0u;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x004081cc, c);
    if ((c.eflags & static_cast<uint32_t>(gp4::flags::CF)) == 0u) {
        GP4_GLOBAL(uint32_t, 0x00a6987c) = 1u;
    }
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00516553, FUN_00516553, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ptr_00679d30")
