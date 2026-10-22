#include <gp4/gp4.h>

// Resets the colour-conversion state (0x00abbc74 = 0, 0x00abbc84 = 0x7fffffff, 0x00abbc88 = 0) and packs
// the 15-bit 5:5:5 colour argument into 0x00abbc7c: each 5-bit field is widened by shifting left 3 (the
// top field keeps the sign-extended upper bits of the argument).
// FUNCTION: GP4 0x004fe4e0
void __stdcall FUN_004fe4e0(uint32_t value) {
    GP4_GLOBAL(int32_t, 0x00abbc84) = 0x7fffffff;
    GP4_GLOBAL(int32_t, 0x00abbc74) = 0;
    GP4_GLOBAL(int32_t, 0x00abbc88) = 0;
    uint32_t top = (value & 0xfffffc00u) | 0xffff8000u;
    uint32_t mid = (top << 3) | (value & 0x3e0u);
    uint32_t low = (mid << 3) | (value & 0x1fu);
    GP4_GLOBAL(uint32_t, 0x00abbc7c) = low << 3;
}
GP4_IMPL(0x004fe4e0, FUN_004fe4e0, "stdcall(u32) -> void; globals=fuzz:i32")
