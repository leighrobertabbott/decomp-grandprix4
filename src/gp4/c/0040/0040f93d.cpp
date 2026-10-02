#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040f93d
void __cdecl FUN_0040f93d(gp4::Regs*) {
    // pushal/popal leaves every register and flag unchanged. Its register
    // saves are private, deallocated stack bytes rather than caller state.
}
GP4_IMPL(0x0040f93d, FUN_0040f93d, "regs() -> ()")
