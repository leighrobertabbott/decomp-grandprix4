#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00433f30
// Scales the two float constants returned by 0x0051faee by the doubles at 0x006f2af4 and
// 0x00601400, rounds them to integers (stored at 0x00630224 / 0x00630228, going through the
// scratch dword 0x006f2bf4), then caches the three table pointers and refreshes the output
// selectors with 0x00433faa.
void __cdecl FUN_00433f30() {
    using namespace gp4::x87;

    gp4::Regs pair = {};
    gp4::call_regs(0x0051faee, pair);

    GP4_GLOBAL(uint32_t, 0x006f2bf4) = pair.eax;
    const Ext first = ext_f32(GP4_GLOBAL(float, 0x006f2bf4))
        * ext(GP4_GLOBAL(double, 0x006f2af4))
        * ext(GP4_GLOBAL(double, 0x00601400));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(first);
    GP4_GLOBAL(int32_t, 0x00630224) = GP4_GLOBAL(int32_t, 0x006f2bf4);

    GP4_GLOBAL(uint32_t, 0x006f2bf4) = pair.edx;
    const Ext second = ext_f32(GP4_GLOBAL(float, 0x006f2bf4))
        * ext(GP4_GLOBAL(double, 0x006f2af4))
        * ext(GP4_GLOBAL(double, 0x00601400));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(second);
    GP4_GLOBAL(int32_t, 0x00630228) = GP4_GLOBAL(int32_t, 0x006f2bf4);

    GP4_GLOBAL(uint32_t, 0x0062ff58) = gp4::cdecl_<0x00434515, uint32_t>();
    GP4_GLOBAL(uint32_t, 0x0062ff5c) = gp4::cdecl_<0x0043452b, uint32_t>();
    GP4_GLOBAL(uint32_t, 0x0062ff60) = gp4::cdecl_<0x00434541, uint32_t>();

    gp4::Regs sel = {};
    gp4::call_regs(0x00433faa, sel);
}
GP4_IMPL(0x00433f30, FUN_00433f30, "cdecl() -> void; globals=fuzz:i32+layout=state_00433faa")
