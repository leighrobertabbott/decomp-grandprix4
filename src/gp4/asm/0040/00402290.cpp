#include <gp4/gp4.h>
#include <gp4/x87.h>

// Converts the three integer globals DAT_00622be8/bec/bf0 to doubles (FILD, then a qword store)
// and writes them to the body block at edx + 0xb8 / 0xc0 / 0xc8.
// FUNCTION: GP4 0x00402290
void __cdecl FUN_00402290(gp4::Regs* r) {
    double* body = (double*)r->edx;

    const int32_t a = GP4_GLOBAL(int32_t, 0x00622be8);
    const int32_t b = GP4_GLOBAL(int32_t, 0x00622bec);
    const int32_t c = GP4_GLOBAL(int32_t, 0x00622bf0);

    GP4_FIELD(double, body, 0xb8) = (double)a;
    GP4_FIELD(double, body, 0xc0) = (double)b;
    GP4_FIELD(double, body, 0xc8) = (double)c;
}
GP4_IMPL(0x00402290, FUN_00402290, "regs(edx:ptr[0xd0]:bytes) -> (); globals=fuzz:i32")
