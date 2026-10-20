#include <gp4/gp4.h>

// Runs the two-argument routine 0x0054c21d (stdcall, `ret 8`) with the string ids 0x8ff and 0x8fe, then
// marks the byte 0x0081d420 with 0xff. eax is the callee's result.
// FUNCTION: GP4 0x0052b3a9
int __cdecl FUN_0052b3a9() {
    const int result = gp4::stdcall_<0x0054c21d, int>(0x8ff, 0x8fe);
    GP4_GLOBAL(uint8_t, 0x0081d420) = 0xffu;
    return result;
}
GP4_IMPL(0x0052b3a9, FUN_0052b3a9, "cdecl() -> i32; globals=fuzz:i32")
