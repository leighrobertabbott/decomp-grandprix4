#include <gp4/gp4.h>

// Setter: stores the argument in the dword global at 0x00a70988 and returns 0.
// FUNCTION: GP4 0x004a5f70
int __cdecl FUN_004a5f70(uint32_t value) {
    GP4_GLOBAL(uint32_t, 0x00a70988) = value;
    return 0;
}
GP4_IMPL(0x004a5f70, FUN_004a5f70, "cdecl(u32) -> i32; globals=fuzz:i32")
