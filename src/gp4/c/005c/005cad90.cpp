#include <gp4/gp4.h>

// Empty stdcall routine with three stack arguments: a single `ret 0xc`.
// FUNCTION: GP4 0x005cad90
void __stdcall FUN_005cad90(int /*a*/, int /*b*/, int /*c*/) {}
GP4_IMPL(0x005cad90, FUN_005cad90, "stdcall(i32, i32, i32) -> void")
