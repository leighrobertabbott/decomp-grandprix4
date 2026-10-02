#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00402305
// Vector (x = DAT_00622c80, y = DAT_00622c88, z = DAT_00622c90, doubles) to two angles:
//   DAT_00622c98 = atan2(y, sqrt(x*x + z*z))   (elevation)
//   DAT_00622ca0 = atan2(x, z)                 (heading)
//   DAT_00622ca8 = 0
// The original keeps the sqrt result on the x87 stack (80-bit) into fpatan, so the
// intermediate chain is computed on extended values and narrowed only at each qword store.
void __cdecl FUN_00402305() {
    using namespace gp4::x87;
    const Ext x = ext(GP4_GLOBAL(double, 0x00622c80));
    const Ext z = ext(GP4_GLOBAL(double, 0x00622c90));
    const Ext y = ext(GP4_GLOBAL(double, 0x00622c88));
    const Ext horizontal = fsqrt(x * x + z * z);
    GP4_GLOBAL(double, 0x00622c98) = narrow(fpatan(y, horizontal));
    GP4_GLOBAL(double, 0x00622ca0) = narrow(fpatan(x, z));
    GP4_GLOBAL(double, 0x00622ca8) = 0.0;
}
GP4_IMPL(0x00402305, FUN_00402305, "cdecl() -> void; globals=fuzz:f64")
