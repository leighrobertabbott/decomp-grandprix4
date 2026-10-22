#include <gp4/gp4.h>

// Returns 1 when the byte at index `index` of the byte table at 0x00b476f3 is nonzero, else 0.
// FUNCTION: GP4 0x005c29a0
int32_t __cdecl FUN_005c29a0(int32_t index) {
    return GP4_ARRAY(uint8_t, 0x00b476f3)[index] != 0;
}
GP4_IMPL(0x005c29a0, FUN_005c29a0, "cdecl(i32[0..7]) -> i32; globals=fuzz:i32")
