#include <gp4/gp4.h>

// Clears the byte 0x00824569 unless it already is zero.
// FUNCTION: GP4 0x0054bad3
void __cdecl FUN_0054bad3() {
    if (GP4_GLOBAL(uint8_t, 0x00824569) != 0u) {
        GP4_GLOBAL(uint8_t, 0x00824569) = 0u;
    }
}
GP4_IMPL(0x0054bad3, FUN_0054bad3, "cdecl() -> void; globals=fuzz:i32")
