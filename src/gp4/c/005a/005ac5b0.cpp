#include <gp4/gp4.h>

// Writes the low byte of `value` to `file` through 0x004d47b0 (passing the address of the
// stack argument and a length of 1).
// FUNCTION: GP4 0x005ac5b0
uint32_t __cdecl FUN_005ac5b0(uint32_t file, uint32_t value) {
    return gp4::cdecl_<0x004d47b0, uint32_t>(file, reinterpret_cast<uint32_t>(&value), 1u);
}
GP4_IMPL(0x005ac5b0, FUN_005ac5b0, "cdecl(u32, u32) -> u32")
