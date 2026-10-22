#include <gp4/gp4.h>

// Returns 1 and pops four stack arguments: `mov eax, 1; ret 0x10`.
// FUNCTION: GP4 0x005ac920
int __stdcall FUN_005ac920(int, int, int, int) { return 1; }
GP4_IMPL(0x005ac920, FUN_005ac920, "stdcall(i32, i32, i32, i32) -> i32")
