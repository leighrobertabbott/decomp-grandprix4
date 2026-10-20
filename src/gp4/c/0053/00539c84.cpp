#include <gp4/gp4.h>

// If the word at 0x007a9470 is zero, sets it to 0x12 and the bytes 0x0081e4fb = 0xff and
// 0x0081e4fe = 0; always sets the byte 0x0082fd5f = 0xff.
// FUNCTION: GP4 0x00539c84
void __cdecl FUN_00539c84(void) {
    if (GP4_GLOBAL(uint16_t, 0x007a9470) == 0u) {
        GP4_GLOBAL(uint16_t, 0x007a9470) = 0x12u;
        GP4_GLOBAL(uint8_t, 0x0081e4fb) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x0081e4fe) = 0u;
    }
    GP4_GLOBAL(uint8_t, 0x0082fd5f) = 0xffu;
}
GP4_IMPL(0x00539c84, FUN_00539c84, "cdecl() -> void; globals=fuzz:i32")
