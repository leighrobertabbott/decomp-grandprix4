#include <gp4/gp4.h>

// Forwards its two stack arguments, in order, to the CRT routine at 0x005dcd83 and returns its result.
// FUNCTION: GP4 0x0046b360
int __stdcall FUN_0046b360(int a, int b) {
    return gp4::cdecl_<0x005dcd83, int>(a, b);
}
GP4_IMPL(0x0046b360, FUN_0046b360, "stdcall(i32, i32) -> i32")
