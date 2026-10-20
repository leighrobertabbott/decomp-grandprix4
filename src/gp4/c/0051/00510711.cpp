#include <gp4/gp4.h>

// pushal/popal wrapper twin of 0x00510700: passes its one stack argument (the slot number, 1..3) in
// eax to the slot-restore routine 0x0051006c and returns. Every register comes back unchanged and the
// caller (0x00481ac0) ignores the flags the callee leaves behind, so this is a plain stdcall.
// FUNCTION: GP4 0x00510711
void __stdcall FUN_00510711(int slot) {
    gp4::Regs c = {};
    c.eax = (uint32_t)slot;
    gp4::call_regs(0x0051006c, c);
}
GP4_IMPL(0x00510711, FUN_00510711, "stdcall(i32[1..3]) -> void; globals=fuzz:i32")
