#include <gp4/gp4.h>
#include <gp4/x87.h>

struct UnknownCoefficients_00411e17 {
    double Scale00;
    double Gain08;
    double Output10;
    double Blend18;
};

// Soft-saturating angle response: with v = st0 * Scale00 and w = v - (v - atan(v)) * Blend18,
// a = atan(w) * Gain08 is stored to DAT_00601480 (qword) and the result left in st0 is
// sin(a) * Output10. Everything stays on the x87 stack (80-bit) until the qword store.
// FUNCTION: GP4 0x00411e17
void __cdecl FUN_00411e17(gp4::Regs* r) {
    using namespace gp4::x87;
    const UnknownCoefficients_00411e17* coefficients = (const UnknownCoefficients_00411e17*)r->eax;

    const Ext input = in_st(r, 0);
    const Ext v = input * ext(coefficients->Scale00);
    const Ext vAtan = fpatan(v, ext_one());
    const Ext correction = (fchs(vAtan) + v) * ext(coefficients->Blend18);
    const Ext w = v + fchs(correction);
    const Ext a = fpatan(w, ext_one()) * ext(coefficients->Gain08);
    GP4_GLOBAL(double, 0x00601480) = narrow(a);
    const Ext result = fsin(a) * ext(coefficients->Output10);

    set_out_st(r, 0, result);
}
GP4_IMPL(0x00411e17, FUN_00411e17, "regs(eax:ptr[0x20]:f64, st0:f64) -> (st0)")
