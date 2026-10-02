#include <gp4/gp4.h>

// FUNCTION: GP4 0x00421eed
void __cdecl FUN_00421eed() {
    GP4_GLOBAL(uint32_t, 0x0060810c) = 0x80000000u;
}
GP4_IMPL(0x00421eed, FUN_00421eed, "cdecl() -> void; globals=fuzz:i32")
