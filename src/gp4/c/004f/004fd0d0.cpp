#include <gp4/gp4.h>

// Stores `value` in slot `index` of the 8-entry table at 0x00aa2d18, but only while the
// counter at 0x0083157c is below 8.
// FUNCTION: GP4 0x004fd0d0
void __stdcall FUN_004fd0d0(uint32_t index, int32_t value) {
    if (GP4_GLOBAL(uint32_t, 0x0083157c) < 8) {
        GP4_ARRAY(int32_t, 0x00aa2d18)[index] = value;
    }
}
GP4_IMPL(0x004fd0d0, FUN_004fd0d0, "stdcall(u32[0..7], i32) -> void; globals=fuzz:i32")
