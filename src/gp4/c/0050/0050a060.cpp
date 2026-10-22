#include <gp4/gp4.h>

// Empty stdcall routine with one argument: `ret 4`.
// FUNCTION: GP4 0x0050a060
void __stdcall FUN_0050a060(int /*a*/) {}
GP4_IMPL(0x0050a060, FUN_0050a060, "stdcall(i32) -> void")
