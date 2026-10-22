#include <gp4/gp4.h>

// Getter: returns the dword global at 0x00a58450.
// FUNCTION: GP4 0x004a19c0
uint32_t __cdecl FUN_004a19c0() {
    return GP4_GLOBAL(uint32_t, 0x00a58450);
}
GP4_IMPL(0x004a19c0, FUN_004a19c0, "cdecl() -> u32; globals=fuzz:i32")
