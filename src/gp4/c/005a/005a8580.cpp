#include <gp4/gp4.h>

// Ignores its first argument, clears the dword the second argument points to and returns 1.
// FUNCTION: GP4 0x005a8580
int32_t __stdcall FUN_005a8580(int32_t /*unused*/, int32_t* out) {
    *out = 0;
    return 1;
}
GP4_IMPL(0x005a8580, FUN_005a8580, "stdcall(i32, ptr[8]:bytes) -> i32")
