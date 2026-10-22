#include <gp4/gp4.h>

// Returns 1 and pops seven stack arguments: `mov eax, 1; ret 0x1c`.
// FUNCTION: GP4 0x005a82e0
int __stdcall FUN_005a82e0(int, int, int, int, int, int, int) { return 1; }
GP4_IMPL(0x005a82e0, FUN_005a82e0, "stdcall(i32, i32, i32, i32, i32, i32, i32) -> i32")
