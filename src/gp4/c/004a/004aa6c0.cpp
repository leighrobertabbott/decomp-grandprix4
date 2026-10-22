#include <gp4/gp4.h>
#include <gp4/x87.h>

// When the byte flag 0x0064b2d0 is set, records a sample ([0x006f2c00] * 100 / [0x007ad8b8], scaled
// by [0x005f04e4]) in the ring 0x00a64c68 (count 0x0064b2bc, index 0x00a64e90). Each time the ring
// wraps, the sample average nudges the level 0x0064b2b8 (above [0x0064b2c8] by rate [0x0064b2c0],
// below [0x0064b2cc] by rate [0x0064b2c4]), clamped to [0, 1], and the ten floats at 0x00a64e68 are
// interpolated between rows floor(2*level) and floor(2*level)+1 (capped at 2) of the 3x10 float table
// 0x0064b2d8. With the flag clear the level is set to 1.0.
// FUNCTION: GP4 0x004aa6c0
void __cdecl FUN_004aa6c0() {
    using namespace gp4::x87;
    if (GP4_GLOBAL(uint8_t, 0x0064b2d0) == 0) {
        GP4_GLOBAL(uint32_t, 0x0064b2b8) = 0x3f800000u;
        return;
    }
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0064b2bc);
    const uint32_t q = (GP4_GLOBAL(uint32_t, 0x006f2c00) * 100u) / GP4_GLOBAL(uint32_t, 0x007ad8b8);
    const uint32_t next = GP4_GLOBAL(uint32_t, 0x00a64e90) + 1u;
    GP4_ARRAY(float, 0x00a64c64)[next] =
        narrow_f32(ext_i64(static_cast<int64_t>(q)) * ext_f32(GP4_GLOBAL(float, 0x005f04e4)));
    const uint32_t idx = next % count;
    GP4_GLOBAL(uint32_t, 0x00a64e90) = idx;
    if (idx != 0u)
        return;

    const Ext zero = ext_f32(GP4_GLOBAL(float, 0x005f0420));
    Ext sum = zero;
    for (uint32_t i = 0; i < count; i++)
        sum = sum + ext_f32(GP4_ARRAY(float, 0x00a64c68)[i]);
    const Ext avg = sum / ext_i32(static_cast<int32_t>(count));

    Ext adj;
    const Ext hi_mark = ext_f32(GP4_GLOBAL(float, 0x0064b2c8));
    if ((fcom(avg, hi_mark) & 0x4100u) == 0u) {
        adj = (hi_mark - avg) * ext_f32(GP4_GLOBAL(float, 0x0064b2c0));
    } else {
        const Ext lo_mark = ext_f32(GP4_GLOBAL(float, 0x0064b2cc));
        const uint16_t sw = static_cast<uint16_t>(fcom(avg, lo_mark) & 0x0500u);
        if (sw == 0x0100u || sw == 0x0400u)
            adj = (lo_mark - avg) * ext_f32(GP4_GLOBAL(float, 0x0064b2c4));
        else
            adj = ext_f32(GP4_GLOBAL(float, 0x005f0420));
    }

    const Ext v = adj + ext_f32(GP4_GLOBAL(float, 0x0064b2b8));
    bool clamp_one = false;
    if ((fcom(ext_f32(GP4_GLOBAL(float, 0x005f0420)), v) & 0x4100u) != 0u) {
        const uint16_t sw = static_cast<uint16_t>(fcom(ext_f32(GP4_GLOBAL(float, 0x005f0428)), v) & 0x0500u);
        if (sw == 0x0100u || sw == 0x0400u)
            clamp_one = true;
    }
    if (clamp_one) {
        GP4_GLOBAL(uint32_t, 0x0064b2b8) = 0x3f800000u;
    } else if ((fcom(ext_f32(GP4_GLOBAL(float, 0x005f0420)), v) & 0x4100u) != 0u) {
        GP4_GLOBAL(float, 0x0064b2b8) = narrow_f32(v);
    } else {
        GP4_GLOBAL(uint32_t, 0x0064b2b8) = 0u;
    }

    const float level = GP4_GLOBAL(float, 0x0064b2b8);
    const Ext twice = ext_f32(level) + ext_f32(level);
    const float twice_f = narrow_f32(twice);
    const double fl = gp4::cdecl_<0x005db88c, double>(narrow(twice));
    gp4::Regs c = {};
    c.st_in = 1;
    set_in_st(c, 0, ext(fl));
    gp4::call_regs(0x005daabc, c);
    const uint32_t row = c.eax;
    const Ext frac = ext_f32(twice_f) - ext_i64(static_cast<int64_t>(row));
    uint32_t hi = row + 1u;
    if (hi > 2u)
        hi = 2u;
    uint32_t lo = row;
    if (lo > 2u)
        lo = 2u;
    const float* a = GP4_ARRAY(float, 0x0064b2d8) + lo * 10u;
    const float* b = GP4_ARRAY(float, 0x0064b2d8) + hi * 10u;
    float* out = GP4_ARRAY(float, 0x00a64e68);
    for (int k = 0; k < 10; k++)
        out[k] = narrow_f32((ext_f32(b[k]) - ext_f32(a[k])) * frac + ext_f32(a[k]));
}
GP4_IMPL(0x004aa6c0, FUN_004aa6c0, "cdecl() -> void; gfields={0x0064b2d0:u8{=0,1,1,1,2}, 0x006f2c00:u32[0..40], 0x007ad8b8:u32[1..40], 0x0064b2bc:u32{=1,2,4,10,128}, 0x00a64e90:u32{=0,1,3,9,127}, 0x00a64c68*128/4:f32[0..2], 0x0064b2b8:f32[-1..2], 0x0064b2c0:f32[-1..1], 0x0064b2c4:f32[-1..1], 0x0064b2c8:f32[0..3], 0x0064b2cc:f32[-1..2], 0x0064b2d8*30/4:f32[-10..10]}")
