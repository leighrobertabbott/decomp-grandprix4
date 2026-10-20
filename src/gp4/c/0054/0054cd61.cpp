#include <gp4/gp4.h>

// Clears the byte flag at 0x00833cf4.
// FUNCTION: GP4 0x0054cd61
void __cdecl FUN_0054cd61() {
    GP4_GLOBAL(uint8_t, 0x00833cf4) = 0u;
}
GP4_IMPL(0x0054cd61, FUN_0054cd61, "cdecl() -> void; globals=fuzz:i32")
