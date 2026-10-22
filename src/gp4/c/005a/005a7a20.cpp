#include <gp4/gp4.h>
#include <gp4/x87.h>

// Arctangent of a float argument, left in st0: `fld dword [esp+4]; fld1; fpatan; ret`
// (atan2(x, 1)).
// FUNCTION: GP4 0x005a7a20
double __cdecl FUN_005a7a20(float x) {
    using namespace gp4::x87;
    return narrow(fpatan(ext_f32(x), ext_one()));
}
GP4_IMPL(0x005a7a20, FUN_005a7a20, "cdecl(f32) -> f64")
