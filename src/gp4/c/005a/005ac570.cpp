#include <gp4/gp4.h>

// Byte-swaps the low 16 bits of `value` in place (the new low byte is bits 8..14, plus 8 when bit 15 is set;
// the new high byte is the old low byte) and writes those 2 bytes to `file` through 0x004d47b0.
// FUNCTION: GP4 0x005ac570
uint32_t __cdecl FUN_005ac570(uint32_t file, uint32_t value) {
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
    const uint8_t lowByte = static_cast<uint8_t>(value);
    uint8_t highBits = static_cast<uint8_t>(static_cast<int32_t>(value) >> 8) & 0x7f;
    if (static_cast<int16_t>(value) < 0) {
        highBits = static_cast<uint8_t>(highBits + 8);
    }
    bytes[1] = lowByte;
    bytes[0] = highBits;
    return gp4::cdecl_<0x004d47b0, uint32_t>(file, reinterpret_cast<uint32_t>(bytes), 2u);
}
GP4_IMPL(0x005ac570, FUN_005ac570, "cdecl(u32, u32) -> u32")
