#include <gp4/gp4.h>

// Stores value * [0x00b1bf80] at 0x00831564, then runs the no-op hook 0x0054633d. The original saves
// and restores eax around the body (push eax ... pop eax), so no register is changed.
// FUNCTION: GP4 0x005462f4
void __stdcall FUN_005462f4(int32_t value) {
    GP4_GLOBAL(int32_t, 0x00831564) = static_cast<int32_t>(
        static_cast<uint32_t>(value) * GP4_GLOBAL(uint32_t, 0x00b1bf80));
    gp4::Regs hook = {};
    gp4::call_regs(0x0054633d, hook);
}
GP4_IMPL(0x005462f4, FUN_005462f4, "stdcall(i32) -> void; globals=fuzz:i32")
