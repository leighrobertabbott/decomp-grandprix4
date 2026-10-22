#include <gp4/gp4.h>

// Stores the second argument as a float in 0x00abbc8c, then forwards the first argument to 0x005b4760
// (a void routine; eax on return is leftover, and the only caller wraps the call in pushal/popal).
// FUNCTION: GP4 0x004fe2c0
void __stdcall FUN_004fe2c0(int a, int b) {
    GP4_GLOBAL(float, 0x00abbc8c) = static_cast<float>(b);
    gp4::cdecl_<0x005b4760, void>(a);
}
GP4_IMPL(0x004fe2c0, FUN_004fe2c0, "stdcall(i32, i32) -> void; globals=fuzz:i32")
