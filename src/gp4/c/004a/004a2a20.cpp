#include <gp4/gp4.h>

// Looks `value` up in the table of 12-byte entries {lo, hi, index} at 0x00a58380 (count at
// 0x00a58b24): the first entry with lo <= value <= hi (unsigned) wins and *out receives the address
// of the 15-dword record 0x00a57fc0 + index * 0x3c, returning 1. When no entry matches *out
// receives the default record 0x00a58ad4 and 0 is returned.
// FUNCTION: GP4 0x004a2a20
uint8_t __cdecl FUN_004a2a20(uint32_t value, uint32_t* out) {
    uint32_t count = GP4_GLOBAL(uint32_t, 0x00a58b24);
    uint32_t i = 0;
    if (count != 0) {
        const uint32_t* hi = GP4_ARRAY(uint32_t, 0x00a58384);
        do {
            if (value >= hi[-1] && value <= hi[0]) {
                uint32_t index = GP4_ARRAY(uint32_t, 0x00a58388)[i * 3];
                *out = 0x00a57fc0 + index * 0x3c;
                return 1;
            }
            i++;
            hi += 3;
        } while (i < count);
    }
    *out = 0x00a58ad4;
    return 0;
}
GP4_IMPL(0x004a2a20, FUN_004a2a20, "cdecl(u32[0..6], ptr[4]:bytes) -> u8; gfields={0x00a58b24:u32[0..4], 0x00a58380*4/0xc:u32[0..4], 0x00a58384*4/0xc:u32[2..6], 0x00a58388*4/0xc:u32[0..3]}")
