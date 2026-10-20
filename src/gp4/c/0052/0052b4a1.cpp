#include <gp4/gp4.h>

// Clears the byte flag at 0x0081d428.
// FUNCTION: GP4 0x0052b4a1
void __cdecl FUN_0052b4a1() {
    GP4_GLOBAL(uint8_t, 0x0081d428) = 0u;
}
GP4_IMPL(0x0052b4a1, FUN_0052b4a1, "cdecl() -> void; globals=fuzz:i32")
