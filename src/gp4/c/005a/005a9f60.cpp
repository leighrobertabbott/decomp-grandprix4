#include <gp4/gp4.h>

// Returns 1 and pops two stack arguments: `mov eax, 1; ret 8`.
// FUNCTION: GP4 0x005a9f60
int __stdcall FUN_005a9f60(int, int) { return 1; }
GP4_IMPL(0x005a9f60, FUN_005a9f60, "stdcall(i32, i32) -> i32")
