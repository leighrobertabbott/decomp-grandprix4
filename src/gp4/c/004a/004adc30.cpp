#include <gp4/gp4.h>

// Clears the dword global at 0x00a66230 and returns 0.
// FUNCTION: GP4 0x004adc30
int32_t __cdecl FUN_004adc30() {
    GP4_GLOBAL(uint32_t, 0x00a66230) = 0;
    return 0;
}
GP4_IMPL(0x004adc30, FUN_004adc30, "cdecl() -> i32; globals=fuzz:i32")
