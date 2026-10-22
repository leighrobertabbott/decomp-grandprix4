#include <gp4/gp4.h>

// Reads a big-endian 16-bit value from two bytes.
// FUNCTION: GP4 0x005b14a0
int32_t __stdcall FUN_005b14a0(const uint8_t* bytes) {
    return (int32_t)bytes[0] * 0x100 + (int32_t)bytes[1];
}
GP4_IMPL(0x005b14a0, FUN_005b14a0, "stdcall(ptr[0x10]:bytes) -> i32")
