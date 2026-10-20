#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;
using gp4::x87::ext;
using gp4::x87::ext_i32;

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit logic operation whose result is `result` (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Low 32 bits of the signed 64-bit product a*b shifted right by 14 (imul; shrd eax, edx, 14).
static inline uint32_t mulShift14(int32_t a, int32_t b) {
    const int64_t product = (int64_t)a * (int64_t)b;
    return (uint32_t)((uint64_t)product >> 14);
}

// Clamps x to [-limit, limit] the way the original's fucompp/fucom ladder does: a negative (or NaN)
// x becomes -min(|x|, limit), otherwise min(x, limit); limit wins whenever the compare is
// "below" or unordered.
static inline Ext clampMagnitude(const Ext& x, const Ext& limit) {
    using namespace gp4::x87;
    if ((fucom(x, ext_zero()) & 0x0100u) != 0u) {
        const Ext magnitude = fabs(x);
        if ((fucom(limit, magnitude) & 0x0100u) != 0u) {
            return fchs(limit);
        }
        return fchs(magnitude);
    }
    if ((fucom(limit, x) & 0x0100u) != 0u) {
        return limit;
    }
    return x;
}

// FUNCTION: GP4 0x0041724d
// ebx = one of the four 0x300-byte wheel records at 0x006380d8, ecx = its index 0..3 (odd = mirrored
// side). Stores the Q14 load product +0x1d8 * +0xe8 * DAT_0062f880 at +0x270 (and its scaled double
// in DAT_00601478), clamps the two slip inputs (+0x290 * +0x288 signed by +0x40, and
// +0x290 * +0x280 * DAT_00601498 * DAT_006014d0[ecx]) to the limits 0x006f2afc / 0x006f2abc, runs the
// tyre-force routines 0x00411d31 / 0x00411e69 / 0x00411f09 on them and stores the rounded forces at
// +0x274, +0x34/+0xe0 and +0x3c/+0x1a0. +0x26c = the larger rounded |angle| * 0x006f29d4 of the two
// DAT_00601480 snapshots; bit 7 of +0x2e is set when it reaches DAT_00601500.
void __cdecl FUN_0041724d(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* wheel = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t index = r->ecx;
    const bool mirrored = (index & 1u) != 0u;

    GP4_FIELD(uint8_t, wheel, 0x2e) &= 0x7fu;

    const uint32_t load0 = mulShift14(GP4_FIELD(int32_t, wheel, 0x1d8), GP4_FIELD(int32_t, wheel, 0xe8));
    const uint32_t load = mulShift14((int32_t)load0, GP4_GLOBAL(int32_t, 0x0062f880));
    GP4_FIELD(uint32_t, wheel, 0x270) = load;
    GP4_GLOBAL(uint32_t, 0x006f2bf4) = load;
    GP4_GLOBAL(double, 0x00601478) = narrow(ext_i32((int32_t)load) * ext(GP4_GLOBAL(double, 0x006f29f4))
                                            * ext(GP4_GLOBAL(double, 0x00601460)));

    Ext slipA = fabs(ext(GP4_FIELD(double, wheel, 0x290)) * ext(GP4_FIELD(double, wheel, 0x288)))
                * ext(GP4_GLOBAL(double, 0x006f2ac4));
    if (GP4_FIELD(int32_t, wheel, 0x40) < 0) {
        slipA = fchs(slipA);
    }
    slipA = clampMagnitude(slipA, ext(GP4_GLOBAL(double, 0x006f2afc)));

    Ext slipB = ext(GP4_FIELD(double, wheel, 0x290)) * ext(GP4_FIELD(double, wheel, 0x280))
                * ext(GP4_GLOBAL(double, 0x00601498));
    slipB = slipB * ext_i32(GP4_ARRAY(int32_t, 0x006014d0)[index]);
    slipB = slipB * ext(GP4_GLOBAL(double, 0x006f29f4));
    slipB = clampMagnitude(slipB, ext(GP4_GLOBAL(double, 0x006f2abc)));
    if (mirrored) {
        slipB = fchs(slipB);
    }

    gp4::Regs call = *r;
    call.st_xin_mask = 0;
    call.st_xout_mask = 0;
    call.st_in = 1;
    call.st_out = 1;
    set_in_st(call, 0, slipB);
    gp4::call_regs(0x00411d31, call);
    Ext force = out_st(call, 0);
    if (mirrored) {
        force = fchs(force);
    }
    GP4_FIELD(int32_t, wheel, 0x274) = fistp32(force * ext(GP4_GLOBAL(double, 0x00601478))
                                               * ext(GP4_GLOBAL(double, 0x00601420)));

    call.st_xin_mask = 0;
    call.st_xout_mask = 0;
    call.st_in = 2;
    call.st_out = 1;
    set_in_st(call, 0, slipB);
    set_in_st(call, 1, slipA);
    gp4::call_regs(0x00411e69, call);
    force = out_st(call, 0);
    if (mirrored) {
        force = fchs(force);
    }
    const int32_t lateral = fistp32(force * ext(GP4_GLOBAL(double, 0x00601478)));
    GP4_FIELD(int32_t, wheel, 0x34) = lateral;
    GP4_FIELD(int32_t, wheel, 0xe0) = lateral;
    GP4_GLOBAL(double, 0x00601488) = narrow(ext(GP4_GLOBAL(double, 0x00601480)));

    call.st_xin_mask = 0;
    call.st_xout_mask = 0;
    call.st_in = 2;
    call.st_out = 1;
    set_in_st(call, 0, slipB);
    set_in_st(call, 1, slipA);
    gp4::call_regs(0x00411f09, call);
    force = out_st(call, 0);
    const int32_t longitudinal = fistp32(force * ext(GP4_GLOBAL(double, 0x00601478)));
    GP4_FIELD(int32_t, wheel, 0x3c) = longitudinal;
    GP4_FIELD(int32_t, wheel, 0x1a0) = longitudinal;
    GP4_GLOBAL(double, 0x00601490) = narrow(ext(GP4_GLOBAL(double, 0x00601480)));

    const int32_t first = fistp32(fabs(ext(GP4_GLOBAL(double, 0x00601488))) * ext(GP4_GLOBAL(double, 0x006f29d4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = first;
    const int32_t second = fistp32(fabs(ext(GP4_GLOBAL(double, 0x00601490))) * ext(GP4_GLOBAL(double, 0x006f29d4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = second;

    int32_t peak = first;
    if (peak < second) {
        peak = second;
    }
    GP4_FIELD(int32_t, wheel, 0x26c) = peak;

    const int32_t threshold = GP4_GLOBAL(int32_t, 0x00601500);
    uint32_t flags = compareFlags32((uint32_t)peak, (uint32_t)threshold);
    if (peak >= threshold) {
        GP4_FIELD(uint8_t, wheel, 0x2e) |= 0x80u;
        flags = logicFlags8(GP4_FIELD(uint8_t, wheel, 0x2e));
    }

    r->eax = (uint32_t)peak;
    r->edx = (uint32_t)second;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041724d, FUN_0041724d, "regs(ebx:ptr[0x298]:f64, ecx:u32[0..3]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tyre_00601510+tyre_0041724d")
