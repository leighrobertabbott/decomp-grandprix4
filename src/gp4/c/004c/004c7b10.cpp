#include <gp4/gp4.h>

// Returns the float global at 0x00a7185c (fld dword [0xa7185c]).
// FUNCTION: GP4 0x004c7b10
float __cdecl FUN_004c7b10() {
    return GP4_GLOBAL(float, 0x00a7185c);
}
GP4_IMPL(0x004c7b10, FUN_004c7b10, "cdecl() -> f32; globals=fuzz:i32")
