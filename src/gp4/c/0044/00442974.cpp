#include <gp4/gp4.h>

// FUNCTION: GP4 0x00442974
void __cdecl FUN_00442974() {
    GP4_GLOBAL(uint32_t, 0x0066d83c) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d898) = 0;
}
GP4_IMPL(0x00442974, FUN_00442974, "cdecl() -> void; globals=fuzz:i32")
