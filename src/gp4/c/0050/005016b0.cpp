#include <gp4/gp4.h>

// Clears the two dword globals at 0x00abc06c and 0x00abc068 (leaves eax = 0).
// FUNCTION: GP4 0x005016b0
int __cdecl FUN_005016b0() {
    int zero = 0;
    GP4_GLOBAL(int, 0x00abc06c) = zero;
    GP4_GLOBAL(int, 0x00abc068) = zero;
    return zero;
}
GP4_IMPL(0x005016b0, FUN_005016b0, "cdecl() -> i32; globals=fuzz:i32")
