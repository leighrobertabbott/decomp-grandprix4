#include <gp4/gp4.h>

// Stores the argument in the global at 0x00a0360c and returns 1.
// FUNCTION: GP4 0x0046aed0
int32_t __cdecl FUN_0046aed0(int32_t value) {
    GP4_GLOBAL(int32_t, 0x00a0360c) = value;
    return 1;
}
GP4_IMPL(0x0046aed0, FUN_0046aed0, "cdecl(i32) -> i32; globals=fuzz:i32")
