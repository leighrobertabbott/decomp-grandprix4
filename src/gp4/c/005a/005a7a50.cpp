#include <gp4/gp4.h>
#include <gp4/x87.h>

// Absolute value of a float argument, returned in st0 (fld dword [esp+4]; fabs).
// FUNCTION: GP4 0x005a7a50
double __cdecl FUN_005a7a50(float x) { return gp4::x87::fabs_((double)x); }
GP4_IMPL(0x005a7a50, FUN_005a7a50, "cdecl(f32) -> f64")
