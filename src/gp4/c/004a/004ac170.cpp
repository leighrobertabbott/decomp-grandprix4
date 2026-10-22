#include <gp4/gp4.h>

// Calls 0x005367b9 (set current directory to one of two roots) with al = 1 and returns its eax.
// FUNCTION: GP4 0x004ac170
int __cdecl FUN_004ac170() {
    gp4::Regs c = {};
    c.eax = 1;
    gp4::call_regs(0x005367b9, c);
    return static_cast<int>(c.eax);
}
GP4_IMPL(0x004ac170, FUN_004ac170, "cdecl() -> i32; globals=fuzz:i32")
