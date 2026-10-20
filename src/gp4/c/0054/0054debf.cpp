#include <gp4/gp4.h>

// Stores 0xff into the byte flag at 0x00838c28.
// FUNCTION: GP4 0x0054debf
void __cdecl FUN_0054debf() {
    GP4_GLOBAL(uint8_t, 0x00838c28) = 0xff;
}
GP4_IMPL(0x0054debf, FUN_0054debf, "cdecl() -> void; globals=fuzz:i32")
