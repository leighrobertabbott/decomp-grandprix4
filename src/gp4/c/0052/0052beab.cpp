#include <gp4/gp4.h>

// Reads the low two bits of the dword at 0x007ab0ec and returns 3 when both are set, else 0.
// FUNCTION: GP4 0x0052beab
uint32_t __cdecl FUN_0052beab() {
    uint32_t bits = GP4_GLOBAL(uint32_t, 0x007ab0ec) & 3u;
    if (bits != 3u) {
        bits = 0u;
    }
    return bits;
}
GP4_IMPL(0x0052beab, FUN_0052beab, "cdecl() -> i32; globals=fuzz:i32")
