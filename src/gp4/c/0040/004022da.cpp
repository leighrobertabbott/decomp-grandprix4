#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x004022da
// DAT_00622bf8 = sqrt(x*x + z*z + y*y) for the doubles x = DAT_00622c80, y = DAT_00622c88,
// z = DAT_00622c90 (the vector length). Every intermediate stays on the x87 stack (80-bit) and is
// narrowed only at the qword store.
void __cdecl FUN_004022da() {
    using namespace gp4::x87;
    const Ext x = ext(GP4_GLOBAL(double, 0x00622c80));
    const Ext z = ext(GP4_GLOBAL(double, 0x00622c90));
    const Ext y = ext(GP4_GLOBAL(double, 0x00622c88));
    GP4_GLOBAL(double, 0x00622bf8) = narrow(fsqrt(x * x + z * z + y * y));
}
GP4_IMPL(0x004022da, FUN_004022da, "cdecl() -> void; globals=fuzz:f64")
