#include <gp4/gp4.h>

// Returns the dword global at 0x00abc114 and clears it when it was nonzero.
// FUNCTION: GP4 0x00504780
int __cdecl FUN_00504780() {
    int value = GP4_GLOBAL(int, 0x00abc114);
    if (value != 0) {
        GP4_GLOBAL(int, 0x00abc114) = 0;
    }
    return value;
}
GP4_IMPL(0x00504780, FUN_00504780, "cdecl() -> i32; globals=fuzz:i32")
