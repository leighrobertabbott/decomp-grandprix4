#include <gp4/gp4.h>

// Reads 2 bytes from the stream `file` through 0x004d4790 into the low half of the argument slot itself and
// returns them as a sign-folded 16-bit value: (low & 0x7f) << 8, minus (low & 0x80) << 8, plus the high byte.
// Only ax is meaningful (the upper half of eax is leftover register content).
// FUNCTION: GP4 0x005ac650
int16_t __cdecl FUN_005ac650(uint32_t file) {
    gp4::cdecl_<0x004d4790, uint32_t>(file, reinterpret_cast<uint32_t>(&file), 2u);
    const uint32_t lo = static_cast<uint8_t>(file);
    const uint32_t hi = static_cast<uint8_t>(file >> 8);
    return static_cast<int16_t>(((lo & 0x7f) << 8) - ((lo & 0x80) << 8) + hi);
}
GP4_IMPL(0x005ac650, FUN_005ac650, "cdecl(u32) -> i16")
