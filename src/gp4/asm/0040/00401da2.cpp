#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00401da2
// Takes the block in EDX filled by 0x00401d80 (sin/cos pairs of three angles at +0x28/+0x30,
// +0x38/+0x40, +0x48/+0x50) and stores the nine rotation-matrix entries at +0x58..+0x80 followed by
// +0x88 = c*b, +0x90 = a (copy of +0x28) and +0x98 = d*b. Two products (e*a and a*f) stay on the
// x87 stack at 80 bits and feed several outputs; every output is stored as a double.
void __cdecl FUN_00401da2(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* o = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));

    const Ext a = ext(GP4_FIELD(double, o, 0x28));
    const Ext b = ext(GP4_FIELD(double, o, 0x30));
    const Ext c = ext(GP4_FIELD(double, o, 0x38));
    const Ext d = ext(GP4_FIELD(double, o, 0x40));
    const Ext e = ext(GP4_FIELD(double, o, 0x48));
    const Ext f = ext(GP4_FIELD(double, o, 0x50));

    const Ext ea = e * a;
    const Ext af = a * f;

    GP4_FIELD(double, o, 0x58) = narrow((c * ea) + (d * f));
    GP4_FIELD(double, o, 0x60) = narrow(fchs(b * e));
    GP4_FIELD(double, o, 0x68) = narrow((d * ea) - (c * f));
    GP4_FIELD(double, o, 0x70) = narrow((d * e) - (c * af));
    GP4_FIELD(double, o, 0x78) = narrow(b * f);
    GP4_FIELD(double, o, 0x80) = narrow(fchs((d * af) + (c * e)));
    GP4_FIELD(double, o, 0x88) = narrow(c * b);
    GP4_FIELD(double, o, 0x90) = narrow(a);
    GP4_FIELD(double, o, 0x98) = narrow(d * b);
}
GP4_IMPL(0x00401da2, FUN_00401da2, "regs(edx:ptr[0xa0]:f64) -> ()")
