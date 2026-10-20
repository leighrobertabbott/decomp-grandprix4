#include <gp4/gp4.h>

// Clears the two dwords at 0x0088f700 and 0x0088f704.
// FUNCTION: GP4 0x00510def
void __cdecl FUN_00510def() {
    GP4_GLOBAL(uint32_t, 0x0088f700) = 0u;
    GP4_GLOBAL(uint32_t, 0x0088f704) = 0u;
}
GP4_IMPL(0x00510def, FUN_00510def, "cdecl() -> void; globals=fuzz:i32")
