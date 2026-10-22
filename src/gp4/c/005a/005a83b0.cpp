#include <gp4/gp4.h>

// Returns one: `mov eax, 1; ret`.
// FUNCTION: GP4 0x005a83b0
int __cdecl FUN_005a83b0() { return 1; }
GP4_IMPL(0x005a83b0, FUN_005a83b0, "cdecl() -> i32")
