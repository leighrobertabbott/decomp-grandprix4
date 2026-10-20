#include <gp4/gp4.h>

// FUNCTION: GP4 0x00442e04
void __cdecl FUN_00442e04() {
    GP4_GLOBAL(uint32_t, 0x00852d3c) = GP4_GLOBAL(uint32_t, 0x0066d81c);
    GP4_GLOBAL(uint8_t, 0x00852d57) = 0;
    GP4_GLOBAL(uint8_t, 0x00852d5c) = 0;
}
GP4_IMPL(0x00442e04, FUN_00442e04, "cdecl() -> void; globals=fuzz:i32")
