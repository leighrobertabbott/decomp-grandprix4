#include <gp4/gp4.h>

// Getter: returns the dword global at 0x00a717cc.
// FUNCTION: GP4 0x004c7760
uint32_t __cdecl FUN_004c7760() {
    return GP4_GLOBAL(uint32_t, 0x00a717cc);
}
GP4_IMPL(0x004c7760, FUN_004c7760, "cdecl() -> u32; globals=fuzz:i32")
