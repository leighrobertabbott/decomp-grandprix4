#include <gp4/gp4.h>

// Packs four clamped values into one dword. Each byte is bits 8..15 of a value derived from the
// globals 0x00967978..0x00967984 (the 0x1fff - x terms are clamped first).
// FUNCTION: GP4 0x0050c922
int __cdecl FUN_0050c922() {
    uint32_t a = 0x1fffu - GP4_GLOBAL(uint32_t, 0x00967984);
    if (static_cast<int32_t>(a) < 0) {
        a = 0xffffffffu;
    } else if (a > 0x2000u) {
        a = 0x2000u;
    }
    uint32_t b = 0x1fffu - GP4_GLOBAL(uint32_t, 0x0096797c);
    if (static_cast<int32_t>(b) < 0) {
        b = 0;
    } else if (b > 0x1fffu) {
        b = 0x1fffu;
    }
    const uint32_t c = (GP4_GLOBAL(uint32_t, 0x00967980) >> 8) & 0xffu;
    const uint32_t d = (GP4_GLOBAL(uint32_t, 0x00967978) >> 8) & 0xffu;
    return static_cast<int>((((a >> 8) & 0xffu) << 24) | (c << 16) | (((b >> 8) & 0xffu) << 8) | d);
}
GP4_IMPL(0x0050c922, FUN_0050c922,
         "cdecl() -> i32; globals=fuzz:i32; gfields={0x00967984:i32[-4..0x2100], 0x0096797c:i32[-4..0x2100]}")
