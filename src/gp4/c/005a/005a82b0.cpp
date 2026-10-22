#include <gp4/gp4.h>

// Ignores its stack argument and returns 1 (mov eax, 1; ret 4).
// FUNCTION: GP4 0x005a82b0
int __stdcall FUN_005a82b0(int /*a*/) { return 1; }
GP4_IMPL(0x005a82b0, FUN_005a82b0, "stdcall(i32) -> i32")
