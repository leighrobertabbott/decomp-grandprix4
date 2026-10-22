#include <gp4/gp4.h>

// Returns 1 and pops three stack arguments: `mov eax, 1; ret 0xc`.
// FUNCTION: GP4 0x005a8350
int __stdcall FUN_005a8350(int, int, int) { return 1; }
GP4_IMPL(0x005a8350, FUN_005a8350, "stdcall(i32, i32, i32) -> i32")
