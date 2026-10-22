#include <gp4/gp4.h>

// Forwards its single stack argument to the CRT routine at 0x005dced4 and returns its result.
// FUNCTION: GP4 0x0046b3c0
int __stdcall FUN_0046b3c0(int arg) {
    return gp4::cdecl_<0x005dced4, int>(arg);
}
GP4_IMPL(0x0046b3c0, FUN_0046b3c0, "stdcall(i32) -> i32")
