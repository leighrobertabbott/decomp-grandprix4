#include <gp4/gp4.h>
#include <gp4/x87.h>

// Base-10 logarithm of a float argument, returned in st0 (fldlg2; fld dword [esp+4]; fyl2x).
// A compiled C function can only return the value rounded to double, hence the tolerance.
// The argument range is kept positive: the emulator's fyl2x does not pop for x <= 0.
// FUNCTION: GP4 0x005a7a40
double __cdecl FUN_005a7a40(float x) {
    using namespace gp4::x87;
    return narrow(fyl2x(ext_lg2(), ext_f32(x)));
}
GP4_IMPL(0x005a7a40, FUN_005a7a40, "cdecl(f32[0.001..1000000]) -> f64; tol=1e-12")
