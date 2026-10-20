#include <gp4/gp4.h>
#include <gp4/x87.h>

// Multiplies the 3-vector at +0xa0 / +0xa8 / +0xb0 by the 3x3 matrix held row by row in the body at
// edx (rows +0x58, +0x70, +0x88), storing the dot products at +0xb8 / +0xc0 / +0xc8. The partial
// sums stay on the x87 stack (80-bit); only the final fstp of each result narrows to a double.
// FUNCTION: GP4 0x00401e2e
void __cdecl FUN_00401e2e(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));

    Ext first = ext(GP4_FIELD(double, body, 0xa0)) * ext(GP4_FIELD(double, body, 0x58));
    first = first + ext(GP4_FIELD(double, body, 0xa8)) * ext(GP4_FIELD(double, body, 0x60));
    first = first + ext(GP4_FIELD(double, body, 0xb0)) * ext(GP4_FIELD(double, body, 0x68));
    GP4_FIELD(double, body, 0xb8) = narrow(first);

    Ext second = ext(GP4_FIELD(double, body, 0xa0)) * ext(GP4_FIELD(double, body, 0x70));
    second = second + ext(GP4_FIELD(double, body, 0xa8)) * ext(GP4_FIELD(double, body, 0x78));
    second = second + ext(GP4_FIELD(double, body, 0xb0)) * ext(GP4_FIELD(double, body, 0x80));
    GP4_FIELD(double, body, 0xc0) = narrow(second);

    Ext third = ext(GP4_FIELD(double, body, 0xa0)) * ext(GP4_FIELD(double, body, 0x88));
    third = third + ext(GP4_FIELD(double, body, 0xa8)) * ext(GP4_FIELD(double, body, 0x90));
    third = third + ext(GP4_FIELD(double, body, 0xb0)) * ext(GP4_FIELD(double, body, 0x98));
    GP4_FIELD(double, body, 0xc8) = narrow(third);
}
GP4_IMPL(0x00401e2e, FUN_00401e2e, "regs(edx:ptr[0xd0]:f64) -> ()")
