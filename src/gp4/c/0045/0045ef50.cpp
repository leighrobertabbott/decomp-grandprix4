#include <gp4/gp4.h>

// Stub taking three stack arguments and returning false in al.
// FUNCTION: GP4 0x0045ef50
uint8_t __stdcall FUN_0045ef50(int a, int b, int c) { (void)a; (void)b; (void)c; return 0; }
GP4_IMPL(0x0045ef50, FUN_0045ef50, "stdcall(i32, i32, i32) -> u8")
