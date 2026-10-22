#include <gp4/gp4.h>

// Constant getter: returns 0x10; the single argument is ignored.
// FUNCTION: GP4 0x00504c50
int __stdcall FUN_00504c50(int /*a*/) { return 0x10; }
GP4_IMPL(0x00504c50, FUN_00504c50, "stdcall(i32) -> i32")
