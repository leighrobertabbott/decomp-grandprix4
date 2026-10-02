#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040f937
void __cdecl FUN_0040f937(gp4::Regs*) {
    // pushal/popal preserves every register and flag. Its saved register bytes
    // belong to the deallocated callee stack and have no caller-visible effect.
}
GP4_IMPL(0x0040f937, FUN_0040f937, "regs() -> ()")
