#include <gp4/gp4.h>

// FUNCTION: GP4 0x00418fca
uint32_t __cdecl FUN_00418fca() {
    return GP4_GLOBAL(uint32_t, 0x00603a64);
}
GP4_IMPL(0x00418fca, FUN_00418fca, "cdecl() -> u32; globals=fuzz:i32")
