#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041a740
// Resets five byte globals: 0x00623ada = 0xff, 0x00623adb = 0, 0x00623ad9 = 0xff,
// 0x006f2f64 = 0, 0x006f2f65 = 0.
void __cdecl FUN_0041a740() {
    GP4_GLOBAL(uint8_t, 0x00623ada) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00623adb) = 0u;
    GP4_GLOBAL(uint8_t, 0x00623ad9) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x006f2f64) = 0u;
    GP4_GLOBAL(uint8_t, 0x006f2f65) = 0u;
}
GP4_IMPL(0x0041a740, FUN_0041a740, "cdecl() -> void; globals=fuzz:i32")
