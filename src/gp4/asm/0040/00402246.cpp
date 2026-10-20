#include <gp4/gp4.h>
#include <gp4/x87.h>

// Converts the three integer globals DAT_00622bdc/be0/be4 to doubles (FILD, then a qword store)
// and writes them to the body block at edx + 0xa0 / 0xa8 / 0xb0.
// FUNCTION: GP4 0x00402246
void __cdecl FUN_00402246(gp4::Regs* r) {
    double* body = (double*)r->edx;

    const int32_t a = GP4_GLOBAL(int32_t, 0x00622bdc);
    const int32_t b = GP4_GLOBAL(int32_t, 0x00622be0);
    const int32_t c = GP4_GLOBAL(int32_t, 0x00622be4);

    GP4_FIELD(double, body, 0xa0) = (double)a;
    GP4_FIELD(double, body, 0xa8) = (double)b;
    GP4_FIELD(double, body, 0xb0) = (double)c;
}
GP4_IMPL(0x00402246, FUN_00402246, "regs(edx:ptr[0xb8]:bytes) -> (); globals=fuzz:i32")
