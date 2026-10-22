#include <gp4/gp4.h>

// Clears the dwords 0x00abbc40, 0x00abbc74 and 0x00abbc78 and packs a 15-bit 5:5:5 colour value into
// the dword 0x00abbc7c: each of the three 5-bit fields is widened by shifting left 3 (the top field
// keeps the sign-extended upper bits of the argument). Same packing as FUN_004fe790.
// FUNCTION: GP4 0x004fe890
void __stdcall FUN_004fe890(uint32_t value) {
    GP4_GLOBAL(uint32_t, 0x00abbc74) = 0;
    GP4_GLOBAL(uint32_t, 0x00abbc40) = 0;
    GP4_GLOBAL(uint32_t, 0x00abbc78) = 0;
    uint32_t top = (value & 0xfffffc00u) | 0xffff8000u;
    uint32_t mid = (top << 3) | (value & 0x3e0u);
    uint32_t low = (mid << 3) | (value & 0x1fu);
    GP4_GLOBAL(uint32_t, 0x00abbc7c) = low << 3;
}
GP4_IMPL(0x004fe890, FUN_004fe890, "stdcall(u32) -> void; globals=fuzz:i32")
