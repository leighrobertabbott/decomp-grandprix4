#include <gp4/gp4.h>

// Sets the two adjacent flag bytes at 0x00828104 and 0x00828103 to 0xff (in that order).
// FUNCTION: GP4 0x00533d2b
void __cdecl FUN_00533d2b() {
    GP4_GLOBAL(uint8_t, 0x00828104) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00828103) = 0xffu;
}
GP4_IMPL(0x00533d2b, FUN_00533d2b, "cdecl() -> void; globals=fuzz:i32")
