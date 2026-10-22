#include <gp4/gp4.h>

// Empty stdcall routine: a single `ret 0x14` (five ignored arguments).
// FUNCTION: GP4 0x00448f80
void __stdcall FUN_00448f80(int, int, int, int, int) {}
GP4_IMPL(0x00448f80, FUN_00448f80, "stdcall(i32, i32, i32, i32, i32) -> void")
