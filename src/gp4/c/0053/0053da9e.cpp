#include <gp4/gp4.h>

// Clears the byte flag at 0x007ab003.
// FUNCTION: GP4 0x0053da9e
void __cdecl FUN_0053da9e() {
    GP4_GLOBAL(uint8_t, 0x007ab003) = 0u;
}
GP4_IMPL(0x0053da9e, FUN_0053da9e, "cdecl() -> void; globals=fuzz:i32")
