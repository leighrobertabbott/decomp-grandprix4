#include <gp4/gp4.h>
#include <gp4/x87.h>

// Dot product of two 3-vectors held in the body at edx: [+0xc0] = [+0xa0]*[+0x70] +
// [+0xa8]*[+0x78] + [+0xb0]*[+0x80]. The partial sums stay on the x87 stack (80-bit); only the
// final fstp narrows to a double.
// FUNCTION: GP4 0x00401eaa
void __cdecl FUN_00401eaa(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));

    Ext sum = ext(GP4_FIELD(double, body, 0xa0)) * ext(GP4_FIELD(double, body, 0x70));
    const Ext second = ext(GP4_FIELD(double, body, 0xa8)) * ext(GP4_FIELD(double, body, 0x78));
    sum = sum + second;
    const Ext third = ext(GP4_FIELD(double, body, 0xb0)) * ext(GP4_FIELD(double, body, 0x80));
    sum = sum + third;
    GP4_FIELD(double, body, 0xc0) = narrow(sum);
}
GP4_IMPL(0x00401eaa, FUN_00401eaa, "regs(edx:ptr[0xc8]:f64) -> ()")
