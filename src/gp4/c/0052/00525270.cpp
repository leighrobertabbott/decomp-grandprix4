#include <gp4/gp4.h>

// FUNCTION: GP4 0x00525270
// Resets a group of flags and counters (inside a pushal/popal pair, so every register and flag is
// kept): the dwords 0x00679bd8, 0x006f39a8, 0x006f39ac, 0x006f3b88, 0x006f3b8c and the bytes
// 0x006f3b78, 0x009e1778, 0x006f3c00, 0x006f3c01, 0x006f3c02 are cleared, the byte 0x006f3b79 is
// set to 0xff.
void __cdecl FUN_00525270(gp4::Regs* /*r*/) {
    GP4_GLOBAL(uint32_t, 0x00679bd8) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f39a8) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f39ac) = 0u;
    GP4_GLOBAL(uint8_t, 0x006f3b78) = 0u;
    GP4_GLOBAL(uint8_t, 0x006f3b79) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x009e1778) = 0u;
    GP4_GLOBAL(uint8_t, 0x006f3c00) = 0u;
    GP4_GLOBAL(uint8_t, 0x006f3c01) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f3b88) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f3b8c) = 0u;
    GP4_GLOBAL(uint8_t, 0x006f3c02) = 0u;
}
GP4_IMPL(0x00525270, FUN_00525270, "regs() -> (); globals=fuzz:i32")
