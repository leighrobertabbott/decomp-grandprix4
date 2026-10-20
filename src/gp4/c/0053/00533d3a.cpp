#include <gp4/gp4.h>

// Marks the two byte slots 0x00828103 and 0x00828105 as unused (0xff).
// FUNCTION: GP4 0x00533d3a
void __cdecl FUN_00533d3a() {
    GP4_GLOBAL(uint8_t, 0x00828103) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00828105) = 0xffu;
}
GP4_IMPL(0x00533d3a, FUN_00533d3a, "cdecl() -> void; globals=fuzz:i32")
