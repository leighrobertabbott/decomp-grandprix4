#include <gp4/gp4.h>

// Returns the 32-bit global at 0x009fb3fc.
// FUNCTION: GP4 0x0045ee60
uint32_t __cdecl FUN_0045ee60() {
    return GP4_GLOBAL(uint32_t, 0x009fb3fc);
}
GP4_IMPL(0x0045ee60, FUN_0045ee60, "cdecl() -> u32; globals=fuzz:i32")
