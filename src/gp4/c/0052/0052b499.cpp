#include <gp4/gp4.h>

// Clears the byte flag at 0x0081d427.
// FUNCTION: GP4 0x0052b499
void __cdecl FUN_0052b499() {
    GP4_GLOBAL(uint8_t, 0x0081d427) = 0u;
}
GP4_IMPL(0x0052b499, FUN_0052b499, "cdecl() -> void; globals=fuzz:i32")
