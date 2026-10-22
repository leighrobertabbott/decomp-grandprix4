#include <gp4/gp4.h>

// Clears the six flag bytes at 0x00a4b430..35 of the 0x218-byte record selected by the index at
// 0x00a4df50, then sets flag byte 3 in record 0 (or, when the byte at 0x007c2e6c is set, in the record
// selected by the count at 0x00645950 minus one).
// FUNCTION: GP4 0x004815e0
void __cdecl FUN_004815e0() {
    uint8_t* flags = &GP4_ARRAY(uint8_t, 0x00a4b430)[GP4_GLOBAL(int, 0x00a4df50) * 0x218];
    flags[0] = 0;
    flags[1] = 0;
    flags[2] = 0;
    flags[3] = 0;
    flags[4] = 0;
    flags[5] = 0;
    int index = 0;
    if (GP4_GLOBAL(uint8_t, 0x007c2e6c) != 0) {
        index = GP4_GLOBAL(int, 0x00645950) - 1;
    }
    GP4_ARRAY(uint8_t, 0x00a4b433)[index * 0x218] = 1;
}
GP4_IMPL(0x004815e0, FUN_004815e0, "cdecl() -> void; gfields={0x00a4df50:i32[0..19], 0x00645950:i32[1..20], 0x007c2e6c:u8{=0,1}}")
