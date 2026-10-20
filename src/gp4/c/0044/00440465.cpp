#include <gp4/gp4.h>

// FUNCTION: GP4 0x00440465
void __cdecl FUN_00440465() {
    GP4_GLOBAL(uint8_t, 0x00622426) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00622426) = 0;
}
GP4_IMPL(0x00440465, FUN_00440465, "cdecl() -> void; globals=fuzz:i32")
