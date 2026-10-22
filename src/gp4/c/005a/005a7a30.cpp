#include <gp4/gp4.h>
#include <gp4/x87.h>

// Square root of a float argument, returned in st0 (fld dword [esp+4]; fsqrt).
// A compiled C function can only return the double-rounded value, while the original leaves the
// 64-bit-mantissa sqrt in st0, hence the tolerance (about 1e-16 relative).
// FUNCTION: GP4 0x005a7a30
double __cdecl FUN_005a7a30(float x) { return gp4::x87::fsqrt((double)x); }
GP4_IMPL(0x005a7a30, FUN_005a7a30, "cdecl(f32) -> f64; tol=1e-12")
