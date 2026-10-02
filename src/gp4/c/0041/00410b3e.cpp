#include <gp4/gp4.h>

// FUNCTION: GP4 0x00410b3e
int __fastcall FUN_00410b3e(int index, const uint8_t* bytes) {
    if (index < 6) {
        int value = bytes[index] & 0x7f;
        if (value > 0x50) {
            value = 0x50;
        }
        return value;
    }
    int mask = (bytes[5] >> 7);
    mask = (mask << 1) | (bytes[4] >> 7);
    mask = (mask << 1) | (bytes[3] >> 7);
    mask = (mask << 1) | (bytes[2] >> 7);
    mask = (mask << 1) | (bytes[1] >> 7);
    mask = (mask << 1) | (bytes[0] >> 7);
    if (mask != 0) {
        mask += 0x11;
    }
    return mask;
}
GP4_IMPL(0x00410b3e, FUN_00410b3e, "fastcall(i32[0..12], ptr[16]:bytes) -> i32")
