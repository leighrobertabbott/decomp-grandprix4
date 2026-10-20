#include <gp4/gp4.h>

// Stores 0xff into the byte flag at 0x007a8e01.
// FUNCTION: GP4 0x0051f2c2
void __cdecl FUN_0051f2c2() {
    GP4_GLOBAL(uint8_t, 0x007a8e01) = 0xff;
}
GP4_IMPL(0x0051f2c2, FUN_0051f2c2, "cdecl() -> void")
