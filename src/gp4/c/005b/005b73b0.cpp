#include <gp4/gp4.h>

// Stores its two stack arguments in the globals at 0x009ed5b8 and 0x009ed5bc (`ret 8`).
// FUNCTION: GP4 0x005b73b0
void __stdcall FUN_005b73b0(int32_t first, int32_t second) {
    GP4_GLOBAL(int32_t, 0x009ed5b8) = first;
    GP4_GLOBAL(int32_t, 0x009ed5bc) = second;
}
GP4_IMPL(0x005b73b0, FUN_005b73b0, "stdcall(i32, i32) -> void")
