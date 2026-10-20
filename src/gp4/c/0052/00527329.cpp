#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

// Register routine (called last by the pushal/popal wrapper 0x005270f5). Steps the track-ring
// record pointer [0x00679d20] by [0x008755f0] records (0x160 bytes), backwards when the word
// 0x00875e92 is negative: inside the pit (at or above the pit start [0x00891500]) it clamps at the
// pit start / the record before the pit end [0x00866ef8]; on the lap it wraps by [0x00866ec4] below
// the lap start [0x00891504] / at the lap end [0x00866ecc]. The record's word +6 (<< 3, minus
// [0x007a8dd8]) goes to 0x00679b64. Then
//   [0x6f2bf4] = fistp(((n / d) + (a / b) * (c * K1)) * K2 * e * K3)
// with n = [0x008755e0] clamped by magnitude to |d|, d = [0x008755e4] (0 -> 1), a = [0x679b64],
// b = [0x00679b68], c = [0x008755f4], e = [0x00875278], K1/K2/K3 the doubles 0x006f2b9c, 0x006f2ae4,
// 0x006f2b7c (80-bit intermediates); [0x009e7f18] = that + [0x00875244].
// edi keeps the record, eax the sum, ecx |d|, edx d; flags of the final add.
// FUNCTION: GP4 0x00527329
void __cdecl FUN_00527329(gp4::Regs* r) {
    using namespace gp4::x87;

    uint32_t rec = GP4_GLOBAL(uint32_t, 0x00679d20);
    const uint32_t step = GP4_GLOBAL(uint32_t, 0x008755f0) * 0x160u;
    const uint32_t pit_start = GP4_GLOBAL(uint32_t, 0x00891500);
    if (GP4_GLOBAL(int16_t, 0x00875e92) < 0) {
        if (rec >= pit_start) {
            rec -= step;
            if (rec < GP4_GLOBAL(uint32_t, 0x00891500)) {
                rec = GP4_GLOBAL(uint32_t, 0x00891500);
            }
        } else {
            rec -= step;
            if (rec < GP4_GLOBAL(uint32_t, 0x00891504)) {
                rec += GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        }
    } else {
        if (rec >= pit_start) {
            rec += step;
            if (rec >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                rec = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        } else {
            rec += step;
            if (rec >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                rec -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        }
    }

    const int32_t height = static_cast<int32_t>(
        (static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, rec, 6))) << 3) -
        GP4_GLOBAL(uint32_t, 0x007a8dd8));
    GP4_GLOBAL(int32_t, 0x00679b64) = height;

    uint32_t num = GP4_GLOBAL(uint32_t, 0x008755e0);
    uint32_t den = GP4_GLOBAL(uint32_t, 0x008755e4);
    if (den == 0u) {
        den = 1u;
    }
    uint32_t mag = den;
    if (static_cast<int32_t>(mag) < 0) {
        mag = 0u - mag;
    }
    if (static_cast<int32_t>(num) < 0) {
        num = 0u - num;
        if (num >= mag) {
            num = mag;
        }
        num = 0u - num;
    } else if (num >= mag) {
        num = mag;
    }

    const Ext ratio = ext_i32(static_cast<int32_t>(num)) / ext_i32(static_cast<int32_t>(den));
    const Ext slope = ext_i32(height) / ext_i32(GP4_GLOBAL(int32_t, 0x00679b68));
    const Ext gain = ext_i32(GP4_GLOBAL(int32_t, 0x008755f4)) * ext(GP4_GLOBAL(double, 0x006f2b9c));
    Ext sum = ratio + slope * gain;
    sum = sum * ext(GP4_GLOBAL(double, 0x006f2ae4));
    sum = sum * ext_i32(GP4_GLOBAL(int32_t, 0x00875278));
    sum = sum * ext(GP4_GLOBAL(double, 0x006f2b7c));
    const int32_t value = fistp32(sum);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = value;

    const uint32_t base = GP4_GLOBAL(uint32_t, 0x00875244);
    const uint32_t total = static_cast<uint32_t>(value) + base;
    GP4_GLOBAL(uint32_t, 0x009e7f18) = total;

    r->edi = rec;
    r->eax = total;
    r->ecx = mag;
    r->edx = den;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(static_cast<uint32_t>(value), base));
}
GP4_IMPL(0x00527329, FUN_00527329, "regs() -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+box_00550798+step_00527329")
