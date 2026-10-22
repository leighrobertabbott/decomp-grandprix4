#include <gp4/gp4.h>

// Descending comparator on the low 16 bits of the dword each pointer refers to:
// -1 when a's value is the larger, 1 when it is the smaller, 0 when equal.
// FUNCTION: GP4 0x004c6e10
int __cdecl FUN_004c6e10(const uint32_t* a, const uint32_t* b) {
    uint32_t x = *a & 0xffff;
    uint32_t y = *b & 0xffff;
    if (y < x) {
        return -1;
    }
    return x < y;
}
GP4_IMPL(0x004c6e10, FUN_004c6e10,
         "cdecl(ptr[8]:u32{0,1,2,0xffff,0x10001,0x20000}, ptr[8]:u32{0,1,2,0xffff,0x10001,0x20000}) -> i32")
