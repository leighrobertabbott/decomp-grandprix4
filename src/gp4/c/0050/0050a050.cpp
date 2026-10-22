#include <gp4/gp4.h>

// Stub taking two stack arguments and returning true (al = 1).
// FUNCTION: GP4 0x0050a050
uint8_t __stdcall FUN_0050a050(int a, int b) { return 1; }
GP4_IMPL(0x0050a050, FUN_0050a050, "stdcall(i32, i32) -> u8")
