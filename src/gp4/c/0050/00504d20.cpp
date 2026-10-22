#include <gp4/gp4.h>

// Loads the dword global at 0x00a6fe68 and sets al to (value != 0); eax[31:8] keep the
// value's own upper bits (test eax, eax ; setne al).
// FUNCTION: GP4 0x00504d20
int __cdecl FUN_00504d20() {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00a6fe68);
    return (int)((value & 0xffffff00u) | (value != 0 ? 1u : 0u));
}
GP4_IMPL(0x00504d20, FUN_00504d20, "cdecl() -> i32; globals=fuzz:i32")
