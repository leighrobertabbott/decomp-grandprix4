#include <gp4/gp4.h>
#include <gp4/x87.h>

// Arcsine of a float argument, left in st0: `fld dword [esp+4]; call __CIasin (0x5ddab0); ret`.
// A compiled C function can only return the double-rounded value, while the original leaves the
// CRT's 64-bit-mantissa result in st0, hence the tolerance (same as the sibling sqrt 0x005a7a30).
// FUNCTION: GP4 0x005a7a10
double __cdecl FUN_005a7a10(float x) {
    using namespace gp4::x87;
    gp4::Regs c = {};
    set_in_st(c, 0, ext_f32(x));
    c.st_in = 1;
    c.st_out = 1;
    gp4::call_regs(0x005ddab0, c);
    return narrow(out_st(c, 0));
}
GP4_IMPL(0x005a7a10, FUN_005a7a10, "cdecl(f32) -> f64; tol=1e-12")
