#include <gp4/gp4.h>

// Stub taking three stack arguments and returning true in al.
// FUNCTION: GP4 0x0045fb10
uint8_t __stdcall FUN_0045fb10(int a, int b, int c) { (void)a; (void)b; (void)c; return 1; }
GP4_IMPL(0x0045fb10, FUN_0045fb10, "stdcall(i32, i32, i32) -> u8")
