#include <gp4/gp4.h>

// Clears the byte flag at 0x0082fd6b.
// FUNCTION: GP4 0x0053da96
void __cdecl FUN_0053da96() {
    GP4_GLOBAL(uint8_t, 0x0082fd6b) = 0u;
}
GP4_IMPL(0x0053da96, FUN_0053da96, "cdecl() -> void; globals=fuzz:i32")
