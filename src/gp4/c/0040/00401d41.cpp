#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00401d41
// Complex product of (st0 + i*st1) and (st2 + i*st3), all kept in extended
// precision on the x87 stack: st0 = a*c - b*d (real), st1 = a*d + b*c
// (imaginary). The four inputs are consumed.
void __cdecl FUN_00401d41(gp4::Regs* r) {
    const gp4::x87::Ext a = gp4::x87::in_st(r, 0);
    const gp4::x87::Ext b = gp4::x87::in_st(r, 1);
    const gp4::x87::Ext c = gp4::x87::in_st(r, 2);
    const gp4::x87::Ext d = gp4::x87::in_st(r, 3);

    const gp4::x87::Ext imag = a * d + b * c;
    const gp4::x87::Ext real = a * c - b * d;

    gp4::x87::set_out_st(r, 0, real);
    gp4::x87::set_out_st(r, 1, imag);
}
GP4_IMPL(0x00401d41, FUN_00401d41, "regs(st0:f64, st1:f64, st2:f64, st3:f64) -> (st0, st1)")
