#include <gp4/gp4.h>
#include <gp4/x87.h>

// Sine of a float argument, left in st0: `fld dword [esp+4]; fsin; ret`.
// FUNCTION: GP4 0x005a79f0
double __cdecl FUN_005a79f0(float angle) {
    using namespace gp4::x87;
    return narrow(fsin(ext_f32(angle)));
}
GP4_IMPL(0x005a79f0, FUN_005a79f0, "cdecl(f32) -> f64")
