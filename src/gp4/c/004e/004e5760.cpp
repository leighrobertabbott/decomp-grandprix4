#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;
using gp4::x87::ext_f32;
using gp4::x87::ext_i32;
using gp4::x87::narrow_f32;

// Subtracts the wrap span [0x005f0f8c] when the value is above [0x005f0f90] (fcom, C3|C0 clear).
static inline Ext FUN_004e5760_Wrap(const Ext& v) {
    if ((gp4::x87::fcom(v, ext_f32(GP4_GLOBAL(float, 0x005f0f90))) & 0x4100u) == 0) {
        return v - ext_f32(GP4_GLOBAL(float, 0x005f0f8c));
    }
    return v;
}

// The computation. d1slot is the original's first-argument slot, where it keeps the first
// squared distance (the other argument slots stay untouched).
static __declspec(noinline) void FUN_004e5760_Body(int index, float x, float z, float* out, float* d1slot) {
    GP4_GLOBAL(uint32_t, 0x00a69878) = GP4_ARRAY(uint32_t, 0x00a955bc)[index];
    gp4::Regs regs = {};
    gp4::call_regs(0x0051fa72, regs);

    const Ext scale = ext_f32(GP4_GLOBAL(float, 0x005f0a38));
    const Ext unit = ext_f32(GP4_GLOBAL(float, 0x005f082c));

    // First pair.
    const float a0 = narrow_f32(ext_i32(GP4_GLOBAL(int32_t, 0x00a6987c)) * scale);
    Ext a1 = ext_i32(GP4_GLOBAL(int32_t, 0x00a69880)) * scale;
    const Ext a0w = FUN_004e5760_Wrap(ext_f32(a0));
    a1 = FUN_004e5760_Wrap(a1);
    const float ax = narrow_f32(a0w * unit);
    const float ay = narrow_f32(a1 * unit);
    const float dx1 = narrow_f32(ext_f32(ax) - ext_f32(x));
    const Ext dy1e = ext_f32(ay) - ext_f32(z);
    const float dy1 = narrow_f32(dy1e);
    const float d1 = narrow_f32(dy1e * ext_f32(dy1) + ext_f32(dx1) * ext_f32(dx1));
    *d1slot = d1;

    // Second pair.
    const float b0 = narrow_f32(ext_i32(GP4_GLOBAL(int32_t, 0x00a69884)) * scale);
    Ext b1 = ext_i32(GP4_GLOBAL(int32_t, 0x00a69888)) * scale;
    const Ext b0w = FUN_004e5760_Wrap(ext_f32(b0));
    b1 = FUN_004e5760_Wrap(b1);
    const float bx = narrow_f32(b0w * unit);
    const Ext bye = b1 * unit;
    const float dx2 = narrow_f32(ext_f32(bx) - ext_f32(x));
    const Ext dy2e = bye - ext_f32(z);
    const float dy2 = narrow_f32(dy2e);
    const Ext d2 = dy2e * ext_f32(dy2) + ext_f32(dx2) * ext_f32(dx2);

    const Ext k = ext_f32(GP4_GLOBAL(float, 0x005f0428));
    if ((gp4::x87::fcom(ext_f32(d1), d2) & 0x0500u) == 0x0100u) {
        out[1] = ay;
        out[0] = ax;
        const Ext r = k / gp4::x87::fsqrt(ext_f32(d1));
        out[2] = narrow_f32(ext_f32(dx1) * r);
        out[3] = narrow_f32(r * ext_f32(dy1));
        return;
    }
    const Ext root = gp4::x87::fsqrt(d2);
    out[0] = bx;
    out[1] = narrow_f32(bye);
    const Ext r = k / root;
    out[2] = narrow_f32(ext_f32(dx2) * r);
    out[3] = narrow_f32(r * ext_f32(dy2));
}

// Loads the record pointer for slot `index` from the 22-entry table 0x00a955bc into 0x00a69878,
// lets 0x0051fa72 copy its linked record's two integer coordinate pairs to 0x00a6987c..0x00a69888,
// converts both pairs to floats (scale, wrap, unit scale) and writes to out the pair nearer to
// (x, z) followed by the unit-scaled offset of that pair from (x, z).
// FUNCTION: GP4 0x004e5760
void __cdecl FUN_004e5760(int index, float x, float z, float* out) {
    FUN_004e5760_Body(index, x, z, out, reinterpret_cast<float*>(&index));
}
GP4_IMPL(0x004e5760, FUN_004e5760, "cdecl(i32[0..21], f32[-1000..1000], f32[-1000..1000], ptr[0x10]:bytes) -> void; gfields={0x00a955bc*22/4:ptr[0x24]:{*bytes, 0x1c:ptr[0x130]:{*bytes, 0x118*4/4:i32{0x7fffffff,0x7fffffc0,0x7fffff00,0x7ffff000,0x7fff0000,0x7ff00000,0x7f000000,0x70000000,0x40000000,-2147483648,-2147483647,-1073741824}}}}")
