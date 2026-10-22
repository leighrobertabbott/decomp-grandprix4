#include <gp4/gp4.h>

// Stores its two stack arguments in the globals at 0x00b2dcec and 0x00b2dcf4.
// FUNCTION: GP4 0x005b4f00
void __cdecl FUN_005b4f00(int32_t first, int32_t second) {
    GP4_GLOBAL(int32_t, 0x00b2dcec) = first;
    GP4_GLOBAL(int32_t, 0x00b2dcf4) = second;
}
GP4_IMPL(0x005b4f00, FUN_005b4f00, "cdecl(i32, i32) -> void; globals=fuzz:i32")
