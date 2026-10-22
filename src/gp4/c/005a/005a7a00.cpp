#include <gp4/gp4.h>
#include <gp4/x87.h>

// Cosine of a float argument, left in st0: `fld dword [esp+4]; fcos; ret`.
// FUNCTION: GP4 0x005a7a00
double __cdecl FUN_005a7a00(float angle) {
    using namespace gp4::x87;
    return narrow(fcos(ext_f32(angle)));
}
GP4_IMPL(0x005a7a00, FUN_005a7a00, "cdecl(f32) -> f64")
