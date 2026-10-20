#include <gp4/gp4.h>

// FUNCTION: GP4 0x0054cd1c
// Returns the (unaligned) dword at 0x00833cd7.
uint32_t __cdecl FUN_0054cd1c() {
    return GP4_GLOBAL(uint32_t, 0x00833cd7);
}
GP4_IMPL(0x0054cd1c, FUN_0054cd1c, "cdecl() -> u32; globals=fuzz:i32")
