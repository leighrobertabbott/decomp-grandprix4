#include <gp4/gp4.h>

// Stores (value * [0x00b1bf40]) / 0x50 (signed 32-bit product, idiv by 0x50) at 0x00831560, then runs
// the no-op hook 0x0054633d. The original saves and restores eax, ebx and edx around the body.
// FUNCTION: GP4 0x005462cb
void __stdcall FUN_005462cb(int32_t value) {
    const int32_t product = static_cast<int32_t>(
        static_cast<uint32_t>(value) * GP4_GLOBAL(uint32_t, 0x00b1bf40));
    GP4_GLOBAL(int32_t, 0x00831560) = product / 0x50;
    gp4::Regs hook = {};
    gp4::call_regs(0x0054633d, hook);
}
GP4_IMPL(0x005462cb, FUN_005462cb, "stdcall(i32) -> void; globals=fuzz:i32")
