#include <gp4/gp4.h>

// Sets the five bytes 0x00679d18..0x00679d1c to 0xff.
// FUNCTION: GP4 0x0051d4e5
void __cdecl FUN_0051d4e5() {
    GP4_GLOBAL(uint8_t, 0x00679d18) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00679d19) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00679d1a) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00679d1b) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00679d1c) = 0xffu;
}
GP4_IMPL(0x0051d4e5, FUN_0051d4e5, "cdecl() -> void; globals=fuzz:i32")
