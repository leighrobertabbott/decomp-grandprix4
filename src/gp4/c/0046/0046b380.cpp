#include <gp4/gp4.h>

// Forwards its single stack argument to the CRT routine at 0x005dce50 and returns its result.
// FUNCTION: GP4 0x0046b380
int __stdcall FUN_0046b380(int arg) {
    return gp4::cdecl_<0x005dce50, int>(arg);
}
GP4_IMPL(0x0046b380, FUN_0046b380, "stdcall(i32) -> i32")
