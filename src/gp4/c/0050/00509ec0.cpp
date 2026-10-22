#include <gp4/gp4.h>

// Empty stdcall routine with two arguments: `ret 8`.
// FUNCTION: GP4 0x00509ec0
void __stdcall FUN_00509ec0(int /*a*/, int /*b*/) {}
GP4_IMPL(0x00509ec0, FUN_00509ec0, "stdcall(i32, i32) -> void")
