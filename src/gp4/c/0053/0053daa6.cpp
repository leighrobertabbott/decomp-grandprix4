#include <gp4/gp4.h>

// Clears the byte flag at 0x007ab003 (same global as 0x0053da9e; many callers).
// FUNCTION: GP4 0x0053daa6
void __cdecl FUN_0053daa6() {
    GP4_GLOBAL(uint8_t, 0x007ab003) = 0u;
}
GP4_IMPL(0x0053daa6, FUN_0053daa6, "cdecl() -> void; globals=fuzz:i32")
