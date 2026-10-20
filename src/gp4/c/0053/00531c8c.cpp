#include <gp4/gp4.h>

// FUNCTION: GP4 0x00531c8c
void __cdecl FUN_00531c8c(gp4::Regs*) {
    // pushal/popal preserves every register and flag. Its saved register bytes
    // belong to the deallocated callee stack and have no caller-visible effect.
}
GP4_IMPL(0x00531c8c, FUN_00531c8c, "regs() -> ()")
