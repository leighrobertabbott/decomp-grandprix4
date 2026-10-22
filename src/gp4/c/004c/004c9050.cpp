#include <gp4/gp4.h>

// Empty stdcall routine with two arguments: `ret 8`.
// FUNCTION: GP4 0x004c9050
void __stdcall FUN_004c9050(int /*a*/, int /*b*/) {}
GP4_IMPL(0x004c9050, FUN_004c9050, "stdcall(i32, i32) -> void")
