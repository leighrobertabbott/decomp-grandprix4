#include <gp4/gp4.h>

// Pushes `value` onto the small dword stack at 0x00abbc40 (depth in 0x00abbc78) and terminates the
// stack with a zero entry after it.
// FUNCTION: GP4 0x004fe970
void __stdcall FUN_004fe970(uint32_t value) {
    uint32_t* stack = GP4_ARRAY(uint32_t, 0x00abbc40);
    uint32_t depth = GP4_GLOBAL(uint32_t, 0x00abbc78);
    stack[depth] = value;
    depth++;
    GP4_GLOBAL(uint32_t, 0x00abbc78) = depth;
    stack[depth] = 0;
}
GP4_IMPL(0x004fe970, FUN_004fe970, "stdcall(u32) -> void; gfields={0x00abbc78:u32[0..12]}")
