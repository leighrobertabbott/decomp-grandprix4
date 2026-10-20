#include <gp4/gp4.h>

// FUNCTION: GP4 0x0052342c
// Clears the ten flag bytes 0x00852d9b..0x00852da4.
void __cdecl FUN_0052342c() {
    GP4_GLOBAL(uint8_t, 0x00852d9b) = 0;
    GP4_GLOBAL(uint8_t, 0x00852d9c) = 0;
    GP4_GLOBAL(uint8_t, 0x00852d9d) = 0;
    GP4_GLOBAL(uint8_t, 0x00852d9e) = 0;
    GP4_GLOBAL(uint8_t, 0x00852d9f) = 0;
    GP4_GLOBAL(uint8_t, 0x00852da0) = 0;
    GP4_GLOBAL(uint8_t, 0x00852da1) = 0;
    GP4_GLOBAL(uint8_t, 0x00852da2) = 0;
    GP4_GLOBAL(uint8_t, 0x00852da3) = 0;
    GP4_GLOBAL(uint8_t, 0x00852da4) = 0;
}
GP4_IMPL(0x0052342c, FUN_0052342c, "cdecl() -> void")
