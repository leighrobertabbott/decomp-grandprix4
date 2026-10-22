#include <gp4/gp4.h>

// Returns the float global at 0x00a71854 (fld dword [0xa71854]).
// FUNCTION: GP4 0x004c7af0
float __cdecl FUN_004c7af0() { return GP4_GLOBAL(float, 0x00a71854); }
GP4_IMPL(0x004c7af0, FUN_004c7af0, "cdecl() -> f32; globals=fuzz:i32")
