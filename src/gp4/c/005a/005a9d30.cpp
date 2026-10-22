#include <gp4/gp4.h>

// Stub taking four dword arguments and returning zero: `xor eax, eax; ret 0x10`.
// FUNCTION: GP4 0x005a9d30
int __stdcall FUN_005a9d30(int /*a*/, int /*b*/, int /*c*/, int /*d*/) { return 0; }
GP4_IMPL(0x005a9d30, FUN_005a9d30, "stdcall(i32, i32, i32, i32) -> i32")
