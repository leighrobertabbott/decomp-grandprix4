#include <gp4/gp4.h>

// Clears the byte flag at 0x0081d426.
// FUNCTION: GP4 0x0052b491
void __cdecl FUN_0052b491() {
    GP4_GLOBAL(uint8_t, 0x0081d426) = 0u;
}
GP4_IMPL(0x0052b491, FUN_0052b491, "cdecl() -> void; globals=fuzz:i32")
