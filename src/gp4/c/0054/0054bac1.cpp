#include <gp4/gp4.h>

// Sets the byte 0x00824569 to 0xff unless it already is.
// FUNCTION: GP4 0x0054bac1
void __cdecl FUN_0054bac1() {
    if (GP4_GLOBAL(uint8_t, 0x00824569) != 0xffu) {
        GP4_GLOBAL(uint8_t, 0x00824569) = 0xffu;
    }
}
GP4_IMPL(0x0054bac1, FUN_0054bac1, "cdecl() -> void; globals=fuzz:i32")
