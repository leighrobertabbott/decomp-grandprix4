#include <gp4/gp4.h>
#include <gp4/x87.h>

// Stores round(DAT_0062ed48 * DAT_006f2aac * DAT_006f2b04 / (double)DAT_006037a0 * DAT_006f2ad4)
// into DAT_00679b48. The whole chain stays on the x87 stack (80-bit) until the fistp.
// FUNCTION: GP4 0x00412408
void __cdecl FUN_00412408() {
    using namespace gp4::x87;
    Ext value = ext(GP4_GLOBAL(double, 0x0062ed48));
    value = value * ext(GP4_GLOBAL(double, 0x006f2aac));
    value = value * ext(GP4_GLOBAL(double, 0x006f2b04));
    const Ext divisor = ext_i32(GP4_GLOBAL(int32_t, 0x006037a0));
    value = value / divisor;
    value = value * ext(GP4_GLOBAL(double, 0x006f2ad4));
    GP4_GLOBAL(int32_t, 0x00679b48) = fistp32(value);
}
GP4_IMPL(0x00412408, FUN_00412408, "cdecl() -> void; globals=fuzz:f64")
