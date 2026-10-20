#include <gp4/gp4.h>

// FUNCTION: GP4 0x0054ba8e
void __cdecl FUN_0054ba8e() {
    GP4_GLOBAL(uint8_t, 0x0082fffc) = 0xffu;
}
GP4_IMPL(0x0054ba8e, FUN_0054ba8e, "cdecl() -> void; globals=fuzz:i32")
