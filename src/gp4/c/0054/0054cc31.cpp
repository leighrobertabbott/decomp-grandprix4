#include <gp4/gp4.h>

// FUNCTION: GP4 0x0054cc31
// Returns the (unaligned) dword at 0x00833cc9.
uint32_t __cdecl FUN_0054cc31() {
    return GP4_GLOBAL(uint32_t, 0x00833cc9);
}
GP4_IMPL(0x0054cc31, FUN_0054cc31, "cdecl() -> u32; globals=fuzz:i32")
