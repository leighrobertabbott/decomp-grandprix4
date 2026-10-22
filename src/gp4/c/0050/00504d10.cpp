#include <gp4/gp4.h>

// Constant getter: returns 0x4b000; the single argument is ignored.
// FUNCTION: GP4 0x00504d10
int __stdcall FUN_00504d10(int /*a*/) { return 0x4b000; }
GP4_IMPL(0x00504d10, FUN_00504d10, "stdcall(i32) -> i32")
