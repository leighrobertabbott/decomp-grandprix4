#include <gp4/gp4.h>

// Resets three dword globals: 0x00a9eb6c = 0, 0x00a9eb70 = 0xbb03126f (the float -0.002) and
// 0x00a9eb74 = 0.
// FUNCTION: GP4 0x004eedd0
void __cdecl FUN_004eedd0() {
    GP4_GLOBAL(uint32_t, 0x00a9eb6c) = 0;
    GP4_GLOBAL(uint32_t, 0x00a9eb70) = 0xbb03126f;
    GP4_GLOBAL(uint32_t, 0x00a9eb74) = 0;
}
GP4_IMPL(0x004eedd0, FUN_004eedd0, "cdecl() -> void; globals=fuzz:i32")
