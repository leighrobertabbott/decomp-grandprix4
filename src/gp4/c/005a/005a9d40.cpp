#include <gp4/gp4.h>

// Stub taking three dword arguments and returning zero: `xor eax, eax; ret 0xc`.
// FUNCTION: GP4 0x005a9d40
int __stdcall FUN_005a9d40(int /*a*/, int /*b*/, int /*c*/) { return 0; }
GP4_IMPL(0x005a9d40, FUN_005a9d40, "stdcall(i32, i32, i32) -> i32")
