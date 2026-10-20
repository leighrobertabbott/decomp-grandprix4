#include <gp4/gp4.h>

// Stores the argument at 0x00831564, then runs the no-op hook 0x0054633d. The original saves and
// restores eax around the body (push eax ... pop eax), so no register is changed.
// FUNCTION: GP4 0x00546327
void __stdcall FUN_00546327(int32_t value) {
    GP4_GLOBAL(int32_t, 0x00831564) = value;
    gp4::Regs hook = {};
    gp4::call_regs(0x0054633d, hook);
}
GP4_IMPL(0x00546327, FUN_00546327, "stdcall(i32) -> void; globals=fuzz:i32")
