#include <gp4/gp4.h>

// Packs a 15-bit 5:5:5 colour value into the dword 0x00669ee8: each of the three 5-bit fields is
// widened by shifting left 3 (the top field keeps the sign-extended upper bits of the argument).
// The first argument is unused.
// FUNCTION: GP4 0x004fe790
void __stdcall FUN_004fe790(int /*unused*/, uint32_t value) {
    uint32_t top = (value & 0xfffffc00u) | 0xffff8000u;
    uint32_t mid = (top << 3) | (value & 0x3e0u);
    uint32_t low = (mid << 3) | (value & 0x1fu);
    GP4_GLOBAL(uint32_t, 0x00669ee8) = low << 3;
}
GP4_IMPL(0x004fe790, FUN_004fe790, "stdcall(i32, u32) -> void; globals=fuzz:i32")
