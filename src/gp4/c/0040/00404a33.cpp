#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00404a33
void __cdecl FUN_00404a33() {
    using namespace gp4::x87;
    uint16_t* table = GP4_ARRAY(uint16_t, 0x00603140);
    for (int i = 0; i <= 0x100; ++i) {
        GP4_GLOBAL(int32_t, 0x006f2bf4) = i;
        Ext arg = ext(static_cast<double>(GP4_GLOBAL(int32_t, 0x006f2bf4)))
                * ext(GP4_GLOBAL(double, 0x006f2abc))
                * ext(GP4_GLOBAL(double, 0x006f2b7c))
                * ext(GP4_GLOBAL(double, 0x006f29b4));
        Ext c = fcos(arg);
        Ext s = fsqrt(fsqrt(c));
        Ext sum = s + fsqrt(s);
        Ext scaled = sum * ext(GP4_GLOBAL(double, 0x006f2a54)) * ext(GP4_GLOBAL(double, 0x006f2b04));
        GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(scaled);
        int32_t v = GP4_GLOBAL(int32_t, 0x006f2bf4);
        if (v < 0) v = 0;
        if (v > 0x4000) v = 0x4000;
        table[i] = static_cast<uint16_t>(v);
    }
}
GP4_IMPL(0x00404a33, FUN_00404a33, "cdecl() -> void; globals=fuzz:f64")
