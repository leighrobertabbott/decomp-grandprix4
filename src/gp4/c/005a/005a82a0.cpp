#include <gp4/gp4.h>

// Ignores its two stack arguments and returns 1 (mov eax, 1; ret 8).
// FUNCTION: GP4 0x005a82a0
int __stdcall FUN_005a82a0(int /*a*/, int /*b*/) { return 1; }
GP4_IMPL(0x005a82a0, FUN_005a82a0, "stdcall(i32, i32) -> i32")
