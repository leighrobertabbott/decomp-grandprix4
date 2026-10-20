#include <gp4/gp4.h>
#include <gp4/x87.h>

struct UnknownBody_0040226b {
    uint8_t Unknown00[0xb8];
    double UnknownB8;
    double UnknownC0;
    double UnknownC8;
};

// Reads three doubles from the body block at edx + 0xb8 / 0xc0 / 0xc8 and stores them rounded
// (FISTP, current rounding mode) to the integer globals DAT_00622be8/bec/bf0.
// FUNCTION: GP4 0x0040226b
void __cdecl FUN_0040226b(gp4::Regs* r) {
    const UnknownBody_0040226b* body = (const UnknownBody_0040226b*)r->edx;

    const double a = body->UnknownB8;
    const double b = body->UnknownC0;
    const double c = body->UnknownC8;

    GP4_GLOBAL(int32_t, 0x00622be8) = gp4::x87::fistp32(a);
    GP4_GLOBAL(int32_t, 0x00622bec) = gp4::x87::fistp32(b);
    GP4_GLOBAL(int32_t, 0x00622bf0) = gp4::x87::fistp32(c);
}
GP4_IMPL(0x0040226b, FUN_0040226b, "regs(edx:ptr[0xd0]:f64) -> (); globals=fuzz:i32")
