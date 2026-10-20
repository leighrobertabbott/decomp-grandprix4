#include <gp4/gp4.h>

// Stores 0xff into the byte flag at 0x0081d425.
// FUNCTION: GP4 0x0052b489
void __cdecl FUN_0052b489() {
    GP4_GLOBAL(uint8_t, 0x0081d425) = 0xff;
}
GP4_IMPL(0x0052b489, FUN_0052b489, "cdecl() -> void; globals=fuzz:i32")
