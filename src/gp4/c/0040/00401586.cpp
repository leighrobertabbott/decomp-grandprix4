#include <gp4/gp4.h>

// FUNCTION: GP4 0x00401586
// Copies the three words at 0x0066d8be/c0/c2 to 0x00629bc4/c6/c8.
void __cdecl FUN_00401586() {
    GP4_GLOBAL(uint16_t, 0x00629bc4) = GP4_GLOBAL(uint16_t, 0x0066d8be);
    GP4_GLOBAL(uint16_t, 0x00629bc6) = GP4_GLOBAL(uint16_t, 0x0066d8c0);
    GP4_GLOBAL(uint16_t, 0x00629bc8) = GP4_GLOBAL(uint16_t, 0x0066d8c2);
}
GP4_IMPL(0x00401586, FUN_00401586, "cdecl() -> void; globals=fuzz:i32")
