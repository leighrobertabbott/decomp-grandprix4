#include <gp4/gp4.h>

// Scales the CRT's clock() (0x005dec4e: milliseconds since start) by the float at 0x005fa1c8 and
// returns the product on the x87 stack (fild dword; fmul dword).
// FUNCTION: GP4 0x005c0260
double __cdecl FUN_005c0260() {
    int ticks = gp4::cdecl_<0x005dec4e, int>();
    return (double)ticks * (double)GP4_GLOBAL(float, 0x005fa1c8);
}
GP4_IMPL(0x005c0260, FUN_005c0260, "cdecl() -> f64; emu=skip:win32 GetSystemTimeAsFileTime")
