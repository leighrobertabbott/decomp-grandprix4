#include <gp4/gp4.h>
#include <gp4/flags.h>
#include <gp4/x87.h>

// When |[0x008705cc]| (the 32-bit absolute value, compared signed) is at least 0xa6e2d, multiplies
// the incoming st0 by sin(a * k1) * k2 / a with a = [0x008705cc] * [0x006f2bdc] (all at extended
// precision, a pi/2^31 scaled angle); otherwise st0 is left alone. eax is preserved
// (push/pop); the flags are those of `cmp eax, 0xa6e2d`.

// FUNCTION: GP4 0x00559dbf
void __cdecl FUN_00559dbf(gp4::Regs* r) {
    using namespace gp4::x87;
    const int32_t value = GP4_GLOBAL(int32_t, 0x008705cc);
    uint32_t magnitude = static_cast<uint32_t>(value);
    if (value < 0) {
        magnitude = 0u - magnitude;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(magnitude, 0x000a6e2du));
    Ext st0 = in_st(r, 0);
    if (static_cast<int32_t>(magnitude) >= 0x000a6e2d) {
        Ext angle = ext_i32(value) * ext(GP4_GLOBAL(double, 0x006f2bdc));
        Ext scaled = fsin(angle * ext(GP4_GLOBAL(double, 0x006f2a54))) * ext(GP4_GLOBAL(double, 0x006f2a64));
        st0 = st0 * (scaled / angle);
    }
    set_out_st(r, 0, st0);
}
GP4_IMPL(0x00559dbf, FUN_00559dbf, "regs(st0:f64) -> (st0, cf, pf, af, zf, sf, of); globals=fuzz:i32")
