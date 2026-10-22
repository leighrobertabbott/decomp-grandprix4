#include <gp4/gp4.h>

// Returns false in al and pops one stack argument: `xor al, al; ret 4`.
// FUNCTION: GP4 0x005cae40
uint8_t __stdcall FUN_005cae40(int /*a*/) { return 0; }
GP4_IMPL(0x005cae40, FUN_005cae40, "stdcall(i32) -> u8")
