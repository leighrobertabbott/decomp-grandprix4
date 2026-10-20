#include <gp4/gp4.h>

// FUNCTION: GP4 0x00546293
// Converts a (column, row)-style pair of coordinates to device units: the first scaled by
// [0xb1bf40] / 0x50 into 0x831560, the second scaled by [0xb1bf80] into 0x831564, then runs the
// no-op hook 0x54633d. The original saves and restores eax, ebx and edx around the body.
void __stdcall FUN_00546293(int32_t first, int32_t second) {
    const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(first) * GP4_GLOBAL(uint32_t, 0x00b1bf40));
    GP4_GLOBAL(int32_t, 0x00831560) = scaled / 0x50;
    GP4_GLOBAL(uint32_t, 0x00831564) = static_cast<uint32_t>(second) * GP4_GLOBAL(uint32_t, 0x00b1bf80);
    gp4::Regs hook = {};
    gp4::call_regs(0x0054633d, hook);
}
GP4_IMPL(0x00546293, FUN_00546293, "stdcall(i32, i32) -> void; globals=fuzz:i32")
