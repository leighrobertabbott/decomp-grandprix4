#include <gp4/gp4.h>

// Empty stdcall routine with one ignored argument: `push ebp; mov ebp, esp; leave; ret 4`.
// FUNCTION: GP4 0x0051c214
void __stdcall FUN_0051c214(int /*unused*/) {}
GP4_IMPL(0x0051c214, FUN_0051c214, "stdcall(i32) -> void")
