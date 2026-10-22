#include <gp4/gp4.h>

// Ignores its stack argument and returns the dword global at 0x00ae6620.
// FUNCTION: GP4 0x00504c30
int __stdcall FUN_00504c30(int /*unused*/) { return GP4_GLOBAL(int, 0x00ae6620); }
GP4_IMPL(0x00504c30, FUN_00504c30, "stdcall(i32) -> i32; globals=fuzz:i32")
