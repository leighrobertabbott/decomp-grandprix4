#include <gp4/gp4.h>

// Returns the low dword of the stream position of the CRT FILE: fgetpos (0x005dddd0) into a
// local 64-bit position, after the empty routine 0x005ae990.
// FUNCTION: GP4 0x004d4770
uint32_t __cdecl FUN_004d4770(void* file) {
    gp4::cdecl_<0x005ae990, void>();
    uint32_t position[2];
    gp4::cdecl_<0x005dddd0, int>(file, position);
    return position[0];
}
GP4_IMPL(0x004d4770, FUN_004d4770, "cdecl(ptr[0x40]:bytes) -> u32")
