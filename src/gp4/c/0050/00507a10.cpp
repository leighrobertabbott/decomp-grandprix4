#include <gp4/gp4.h>

// Passes its argument in eax to the routine 0x00538a24 and returns its eax.
// FUNCTION: GP4 0x00507a10
int __cdecl FUN_00507a10(int value) {
    gp4::Regs c = {};
    c.eax = static_cast<uint32_t>(value);
    gp4::call_regs(0x00538a24, c);
    return static_cast<int>(c.eax);
}
GP4_IMPL(0x00507a10, FUN_00507a10, "cdecl(i32) -> i32; globals=fuzz:i32")
