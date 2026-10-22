#include <gp4/gp4.h>

// Clears the six flag bytes at 0x00a4b430..35 of the 0x218-byte record selected by the index at
// 0x00a4df50, then sets flag byte 0 in record 0 (or, when the byte at 0x007c2e6c is set, in the record
// selected by the count at 0x00645950 minus one). The original leaves eax = 33 * that record index.
// FUNCTION: GP4 0x004814c0
int __cdecl FUN_004814c0() {
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
    GP4_ARRAY(uint8_t, 0x00a4b430)[index * 0x218] = 1;
    return index * 33;
}
GP4_IMPL(0x004814c0, FUN_004814c0, "cdecl() -> i32; gfields={0x00a4df50:i32[0..3], 0x00645950:i32[1..4], 0x007c2e6c:u8{=0,1}}")
