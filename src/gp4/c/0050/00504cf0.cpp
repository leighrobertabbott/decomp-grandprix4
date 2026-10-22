#include <gp4/gp4.h>

// Getter: returns the dword at +0 of the 0x24-byte entry `index` of the table at 0x0066b9dc.
// FUNCTION: GP4 0x00504cf0
int __stdcall FUN_00504cf0(int index) {
    const uint8_t* entry = GP4_ARRAY(uint8_t, 0x0066b9dc) + index * 0x24;
    return *(const int*)entry;
}
GP4_IMPL(0x00504cf0, FUN_00504cf0, "stdcall(i32[0..15]) -> i32; globals=fuzz:i32")
