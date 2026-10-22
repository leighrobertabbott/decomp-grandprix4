#include <gp4/gp4.h>

// Tests whether the argument is the address 0x00a58ad4.
// FUNCTION: GP4 0x004a2a90
uint8_t __cdecl FUN_004a2a90(const void* p) { return p == GP4_ARRAY(const uint32_t, 0x00a58ad4); }
GP4_IMPL(0x004a2a90, FUN_004a2a90, "cdecl(u32{0x00a58ad4,0,1,0x00a58ad8}) -> u8")
