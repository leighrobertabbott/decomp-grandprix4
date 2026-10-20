#include <gp4/gp4.h>
#include <gp4/x87.h>

struct UnknownBody_004022b5 {
    uint8_t Unknown00[0xa0];
    double UnknownA0;
    double UnknownA8;
    double UnknownB0;
};

// Reads three doubles from the body block at edx + 0xa0 / 0xa8 / 0xb0 and stores them rounded
// (FISTP, current rounding mode) to the integer globals DAT_00622bdc/be0/be4.
// FUNCTION: GP4 0x004022b5
void __cdecl FUN_004022b5(gp4::Regs* r) {
    const UnknownBody_004022b5* body = (const UnknownBody_004022b5*)r->edx;

    const double a = body->UnknownA0;
    const double b = body->UnknownA8;
    const double c = body->UnknownB0;

    GP4_GLOBAL(int32_t, 0x00622bdc) = gp4::x87::fistp32(a);
    GP4_GLOBAL(int32_t, 0x00622be0) = gp4::x87::fistp32(b);
    GP4_GLOBAL(int32_t, 0x00622be4) = gp4::x87::fistp32(c);
}
GP4_IMPL(0x004022b5, FUN_004022b5, "regs(edx:ptr[0xb8]:f64) -> (); globals=fuzz:i32")
