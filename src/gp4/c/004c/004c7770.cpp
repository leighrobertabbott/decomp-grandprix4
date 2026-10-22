#include <gp4/gp4.h>

// Returns the 32-bit global at 0x00a717bc.
// FUNCTION: GP4 0x004c7770
uint32_t __cdecl FUN_004c7770() {
    return GP4_GLOBAL(uint32_t, 0x00a717bc);
}
GP4_IMPL(0x004c7770, FUN_004c7770, "cdecl() -> u32; globals=fuzz:i32")
