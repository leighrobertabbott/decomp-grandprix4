#include <gp4/gp4.h>
#include <gp4/x87.h>

// Rotates the vector (+0xb8, +0xc0, +0xc8) by the 3x3 matrix whose columns are stored at +0x58,
// +0x70 and +0x88 (rows at +0x58/+0x60/+0x68, +0x70/+0x78/+0x80, +0x88/+0x90/+0x98), writing the
// result to +0xa0, +0xa8, +0xb0. Each sum stays on the x87 stack (80-bit); only the final fstp
// narrows to a double. Same code as 0x00401f02.
// FUNCTION: GP4 0x0051eecd
void __cdecl FUN_0051eecd(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));

    const Ext inX = ext(GP4_FIELD(double, body, 0xb8));
    const Ext inY = ext(GP4_FIELD(double, body, 0xc0));
    const Ext inZ = ext(GP4_FIELD(double, body, 0xc8));

    Ext sumX = inX * ext(GP4_FIELD(double, body, 0x58));
    sumX = sumX + inY * ext(GP4_FIELD(double, body, 0x70));
    sumX = sumX + inZ * ext(GP4_FIELD(double, body, 0x88));
    GP4_FIELD(double, body, 0xa0) = narrow(sumX);

    Ext sumY = inX * ext(GP4_FIELD(double, body, 0x60));
    sumY = sumY + inY * ext(GP4_FIELD(double, body, 0x78));
    sumY = sumY + inZ * ext(GP4_FIELD(double, body, 0x90));
    GP4_FIELD(double, body, 0xa8) = narrow(sumY);

    Ext sumZ = inX * ext(GP4_FIELD(double, body, 0x68));
    sumZ = sumZ + inY * ext(GP4_FIELD(double, body, 0x80));
    sumZ = sumZ + inZ * ext(GP4_FIELD(double, body, 0x98));
    GP4_FIELD(double, body, 0xb0) = narrow(sumZ);
}
GP4_IMPL(0x0051eecd, FUN_0051eecd, "regs(edx:ptr[0xd0]:f64) -> ()")
