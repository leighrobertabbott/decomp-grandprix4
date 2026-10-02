#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040f93a
void __cdecl FUN_0040f93a(gp4::Regs*) {
    // pushal/popal restores every register without changing flags. The saved
    // register bytes occupy private stack space deallocated before return.
}
GP4_IMPL(0x0040f93a, FUN_0040f93a, "regs() -> ()")
