#include <gp4/gp4.h>

// Getter: returns the dword global at 0x00a66230.
// FUNCTION: GP4 0x004adc40
uint32_t __cdecl FUN_004adc40() {
    return GP4_GLOBAL(uint32_t, 0x00a66230);
}
GP4_IMPL(0x004adc40, FUN_004adc40, "cdecl() -> u32; globals=fuzz:i32")
