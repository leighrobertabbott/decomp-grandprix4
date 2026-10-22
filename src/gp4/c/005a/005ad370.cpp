#include <gp4/gp4.h>

// Returns seven: `mov eax, 7; ret`.
// FUNCTION: GP4 0x005ad370
int __cdecl FUN_005ad370() { return 7; }
GP4_IMPL(0x005ad370, FUN_005ad370, "cdecl() -> i32")
