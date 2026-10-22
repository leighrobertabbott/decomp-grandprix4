#include <gp4/gp4.h>

// Returns the byte at offset 0 of the 0x24-byte record `index` in the table at 0x0066b9d0,
// zero-extended to eax.
// FUNCTION: GP4 0x00504d30
int __stdcall FUN_00504d30(int index) {
    return (int)GP4_ARRAY(uint8_t, 0x0066b9d0)[index * 0x24];
}
GP4_IMPL(0x00504d30, FUN_00504d30, "stdcall(i32[0..63]) -> i32")
