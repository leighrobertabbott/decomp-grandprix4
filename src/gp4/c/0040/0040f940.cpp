#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040f940
void __cdecl FUN_0040f940(gp4::Regs*) {
    // pushal/popal preserves all registers and flags. Its saved register
    // bytes are on private stack space discarded before control returns.
}
GP4_IMPL(0x0040f940, FUN_0040f940, "regs() -> ()")
