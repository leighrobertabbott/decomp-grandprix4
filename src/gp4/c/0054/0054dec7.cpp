#include <gp4/gp4.h>

// FUNCTION: GP4 0x0054dec7
void __cdecl FUN_0054dec7() {
    GP4_GLOBAL(uint8_t, 0x00838c28) = 0xffu;
}
GP4_IMPL(0x0054dec7, FUN_0054dec7, "cdecl() -> void; globals=fuzz:i32")
