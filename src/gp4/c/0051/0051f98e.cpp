#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051f98e
void __cdecl FUN_0051f98e(gp4::Regs*) {
    // pushal/popal preserves every register and flag. Its saved register bytes
    // belong to the deallocated callee stack and have no caller-visible effect.
}
GP4_IMPL(0x0051f98e, FUN_0051f98e, "regs() -> ()")
