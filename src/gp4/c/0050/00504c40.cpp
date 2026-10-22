#include <gp4/gp4.h>

// Returns the global dword at 0x00ae6624; the single argument is ignored.
// FUNCTION: GP4 0x00504c40
int __stdcall FUN_00504c40(int /*a*/) { return GP4_GLOBAL(int, 0x00ae6624); }
GP4_IMPL(0x00504c40, FUN_00504c40, "stdcall(i32) -> i32; globals=fuzz:i32")
