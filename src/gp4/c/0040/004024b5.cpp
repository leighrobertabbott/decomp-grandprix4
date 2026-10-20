#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x004024b5
// Three cross-product-like terms kept on the x87 stack at extended precision:
//   a = (D6097a0 - D609798) * D609770 * D609768 * D6f2bdc / D609790   -> D609778
//   b = (D609790 - D6097a0) * D609760 * D609770 * D6f2bdc / D609798   -> D609780
//   c = (D609798 - D609790) * D609768 * D609760 * D6f2bdc / D6097a0   -> D609788
// and, when the vector length is not below the int32 at 0x609758, the vector is
// rescaled to that length.  The final "ficom; fnstsw ax; sahf" leaves CF/PF/ZF
// (the original never touches OF; AF/SF come from the TOP/B bits, always 0).
void __cdecl FUN_004024b5(gp4::Regs* r) {
    using namespace gp4::x87;
    Ext a = (((ext(GP4_GLOBAL(double, 0x006097a0)) - ext(GP4_GLOBAL(double, 0x00609798)))
        * ext(GP4_GLOBAL(double, 0x00609770))) * ext(GP4_GLOBAL(double, 0x00609768)))
        * ext(GP4_GLOBAL(double, 0x006f2bdc)) / ext(GP4_GLOBAL(double, 0x00609790));
    Ext b = (((ext(GP4_GLOBAL(double, 0x00609790)) - ext(GP4_GLOBAL(double, 0x006097a0)))
        * ext(GP4_GLOBAL(double, 0x00609760))) * ext(GP4_GLOBAL(double, 0x00609770)))
        * ext(GP4_GLOBAL(double, 0x006f2bdc)) / ext(GP4_GLOBAL(double, 0x00609798));
    Ext c = (((ext(GP4_GLOBAL(double, 0x00609798)) - ext(GP4_GLOBAL(double, 0x00609790)))
        * ext(GP4_GLOBAL(double, 0x00609768))) * ext(GP4_GLOBAL(double, 0x00609760)))
        * ext(GP4_GLOBAL(double, 0x006f2bdc)) / ext(GP4_GLOBAL(double, 0x006097a0));

    const Ext length = fsqrt((a * a) + ((b * b) + (c * c)));
    const int32_t limit = GP4_GLOBAL(int32_t, 0x00609758);
    const uint16_t status = fcom(length, ext_i32(limit));
    if ((status & 0x0100u) == 0u) {
        const Ext ratio = ext_i32(limit) / length;
        c = c * ratio;
        b = b * ratio;
        a = a * ratio;
    }
    GP4_GLOBAL(double, 0x00609788) = narrow(c);
    GP4_GLOBAL(double, 0x00609780) = narrow(b);
    GP4_GLOBAL(double, 0x00609778) = narrow(a);

    const uint32_t flags = ((status & 0x0100u) != 0u ? 1u : 0u)
        | ((status & 0x0400u) != 0u ? 4u : 0u)
        | ((status & 0x4000u) != 0u ? 0x40u : 0u);
    r->eflags = (r->eflags & ~0xd5u) | flags;
}
GP4_IMPL(0x004024b5, FUN_004024b5, "regs() -> (cf, pf, af, zf, sf); globals=fuzz:f64")
