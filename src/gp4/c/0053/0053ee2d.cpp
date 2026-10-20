#include <gp4/gp4.h>

// Calls the empty routine 0x00531b3f, then stores 0xff into the byte flag at 0x0082ff13.
// FUNCTION: GP4 0x0053ee2d
void __cdecl FUN_0053ee2d() {
    gp4::cdecl_<0x00531b3f, void>();
    GP4_GLOBAL(uint8_t, 0x0082ff13) = 0xff;
}
GP4_IMPL(0x0053ee2d, FUN_0053ee2d, "cdecl() -> void; globals=fuzz:i32")
