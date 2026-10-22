#include <gp4/gp4.h>

// Writes the low 16 bits of `value` big-endian to two bytes.
// FUNCTION: GP4 0x005b14c0
void __stdcall FUN_005b14c0(uint8_t* bytes, uint32_t value) {
    bytes[0] = (uint8_t)(value >> 8);
    bytes[1] = (uint8_t)value;
}
GP4_IMPL(0x005b14c0, FUN_005b14c0, "stdcall(ptr[0x10]:bytes, u32) -> void")
