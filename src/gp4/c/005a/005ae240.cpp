#include <gp4/gp4.h>

// Returns true (1) in eax and pops one stack argument: `mov eax, 1; ret 4`.
// FUNCTION: GP4 0x005ae240
int __stdcall FUN_005ae240(int /*a*/) { return 1; }
GP4_IMPL(0x005ae240, FUN_005ae240, "stdcall(i32) -> i32")
