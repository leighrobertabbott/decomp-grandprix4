#include <gp4/gp4.h>

// pushal/popal wrapper: passes its one stack argument (the slot number, 1..3) in eax to the
// slot-save routine 0x00510309 and returns. Every register comes back unchanged and the callers
// ignore the flags the callee leaves behind, so this is a plain stdcall with a void result.
// FUNCTION: GP4 0x00510700
void __stdcall FUN_00510700(int slot) {
    gp4::Regs c = {};
    c.eax = (uint32_t)slot;
    gp4::call_regs(0x00510309, c);
}
GP4_IMPL(0x00510700, FUN_00510700, "stdcall(i32[1..3]) -> void; globals=layout=save_00510309")
