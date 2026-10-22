#include <gp4/gp4.h>

// Returns the float global at 0x00a71858 (fld dword [0xa71858]).
// FUNCTION: GP4 0x004c7b00
float __cdecl FUN_004c7b00() {
    return GP4_GLOBAL(float, 0x00a71858);
}
GP4_IMPL(0x004c7b00, FUN_004c7b00, "cdecl() -> f32; globals=fuzz:i32")
