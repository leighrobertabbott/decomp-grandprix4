#include <gp4/gp4.h>

// Clears the two dwords at 0x00874ef0 and 0x00874ee4.
// FUNCTION: GP4 0x00512b11
void __cdecl FUN_00512b11() {
    GP4_GLOBAL(uint32_t, 0x00874ef0) = 0u;
    GP4_GLOBAL(uint32_t, 0x00874ee4) = 0u;
}
GP4_IMPL(0x00512b11, FUN_00512b11, "cdecl() -> void; globals=fuzz:i32")
