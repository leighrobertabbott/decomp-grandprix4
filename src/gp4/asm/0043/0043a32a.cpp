#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `left + right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// Negated angle from 0x00401315, magnitude clamped to 0x3000.
static inline uint32_t clampAngle(uint32_t angle) {
    uint32_t v = 0u - angle;
    if ((int32_t)v < 0) {
        uint32_t m = 0u - v;
        if (m >= 0x3000u) m = 0x3000u;
        return 0u - m;
    }
    if (v >= 0x3000u) v = 0x3000u;
    return v;
}

// One side of the bounds: the fraction (Q14, 0..0x4000) of the way from the negative-side
// sample `neg` to the positive-side sample `pos` at which zero lies, applied between the two
// extreme values `lo` (negative side) and `hi` (positive side).
static inline uint32_t sideAtZero(int32_t pos, int32_t neg, int32_t hi, int32_t lo) {
    int32_t den = (int32_t)((uint32_t)pos - (uint32_t)neg);
    if (den == 0) den = 1;
    int32_t rem;
    int32_t q = gp4::idiv64((int64_t)(int32_t)(0u - (uint32_t)neg) << 14, den, &rem);
    if (q < 0) q = 0;
    if (q > 0x4000) q = 0x4000;
    const int64_t p = (int64_t)(int32_t)((uint32_t)hi - (uint32_t)lo) * (int64_t)q;
    return (uint32_t)((uint64_t)p >> 14) + (uint32_t)lo;
}

// Rotates (x, y, z) by the matrix of `body` (0x00401f02) and rounds the result into the
// three sample slots 0x00608290 / 0x00608294 / 0x00608298.
static __declspec(noinline) void rotateSample(gp4::Regs& c, uint8_t* body, int32_t x, int32_t y, int32_t z) {
    GP4_FIELD(double, body, 0xb8) = (double)x;
    GP4_FIELD(double, body, 0xc0) = (double)y;
    GP4_FIELD(double, body, 0xc8) = (double)z;
    c.edx = (uint32_t)(uintptr_t)body;
    gp4::call_regs(0x00401f02, c);
    GP4_GLOBAL(int32_t, 0x00608290) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa0));
    GP4_GLOBAL(int32_t, 0x00608294) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa8));
    GP4_GLOBAL(int32_t, 0x00608298) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb0));
}

// Rotates ([0x60829c], [0x60829c], [0x608298]) by `body` into 0x00623e30 / 34 / 38.
static __declspec(noinline) void rotateCentre(gp4::Regs& c, uint8_t* body) {
    GP4_FIELD(double, body, 0xb8) = (double)GP4_GLOBAL(int32_t, 0x0060829c);
    GP4_FIELD(double, body, 0xc0) = (double)GP4_GLOBAL(int32_t, 0x0060829c);
    GP4_FIELD(double, body, 0xc8) = (double)GP4_GLOBAL(int32_t, 0x00608298);
    c.edx = (uint32_t)(uintptr_t)body;
    gp4::call_regs(0x00401f02, c);
    GP4_GLOBAL(int32_t, 0x00623e30) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa0));
    GP4_GLOBAL(int32_t, 0x00623e34) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa8));
    GP4_GLOBAL(int32_t, 0x00623e38) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb0));
}

// Builds the orientation body from the record's three angles (0x401d5c / 0x401d80 / 0x401da2),
// transforms the record vector +0x18..+0x20 with it (0x401e2e) and rounds the result into
// 0x00608290 / 94 / 98.
static __declspec(noinline) void orientRecord(gp4::Regs& c, uint8_t* body, const int32_t* angles, const int32_t* vec) {
    GP4_FIELD(int32_t, body, 0x0) = angles[0];
    GP4_FIELD(int32_t, body, 0x4) = angles[1];
    GP4_FIELD(int32_t, body, 0x8) = angles[2];
    c.edx = (uint32_t)(uintptr_t)body;
    gp4::call_regs(0x00401d5c, c);
    gp4::call_regs(0x00401d80, c);
    gp4::call_regs(0x00401da2, c);
    GP4_FIELD(double, body, 0xa0) = (double)vec[0];
    GP4_FIELD(double, body, 0xa8) = (double)vec[1];
    GP4_FIELD(double, body, 0xb0) = (double)vec[2];
    gp4::call_regs(0x00401e2e, c);
    GP4_GLOBAL(int32_t, 0x00608290) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb8));
    GP4_GLOBAL(int32_t, 0x00608294) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc0));
    GP4_GLOBAL(int32_t, 0x00608298) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc8));
}

// esi = collision object record (byte +0x6d = shape block index). Samples every point of the
// object's shape block (0x6099c0 + idx*0x5d8, 0x14-byte points from +0x38, count +0x18) through
// the body 0x6232a8 into the four side bounds 0x609800..0x60981c (0x43a148; mode byte 0x6097fc:
// nonzero tracks maxima, zero minima). If every side was reached, derives two clamped angles,
// the orientation (0x43a079) and a contact point and fills the 0x34-byte record at [0x60982c];
// CF clear. Otherwise CF set. pushal/popal: all registers are preserved.
// FUNCTION: GP4 0x0043a32a
void __cdecl FUN_0043a32a(gp4::Regs* r) {
    uint8_t* const obj = (uint8_t*)r->esi;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint8_t* const blk = GP4_ARRAY(uint8_t, 0x006099c0) + (uint32_t)GP4_FIELD(uint8_t, obj, 0x6d) * 0x5d8u;
    const int32_t* const head = (const int32_t*)blk;

    const int32_t start = GP4_GLOBAL(uint8_t, 0x006097fc) != 0 ? (int32_t)0x80000000 : 0x7fffffff;
    GP4_GLOBAL(int32_t, 0x00609808) = start;
    GP4_GLOBAL(int32_t, 0x0060980c) = start;
    GP4_GLOBAL(int32_t, 0x00609818) = start;
    GP4_GLOBAL(int32_t, 0x0060981c) = start;

    const int32_t* pt = (const int32_t*)(blk + 0x38);
    uint32_t i = 0;
    do {
        if (pt[3] == 1 || pt[3] == 2) {
            c.ebx = (uint32_t)(uintptr_t)blk;
            c.edi = (uint32_t)(uintptr_t)pt;
            c.ecx = i;
            c.edx = pt[3] == 1 ? (uint32_t)(uintptr_t)GP4_ARRAY(uint8_t, 0x00607e40)
                               : (uint32_t)(uintptr_t)GP4_ARRAY(uint8_t, 0x00607dc0);
            gp4::call_regs(0x0043a295, c);
        } else {
            const int32_t x = (int32_t)((uint32_t)pt[0] - (uint32_t)head[0]);
            GP4_GLOBAL(int32_t, 0x00623e18) = x;
            const int32_t y = (int32_t)((uint32_t)pt[1] - (uint32_t)head[1]);
            GP4_GLOBAL(int32_t, 0x00623e1c) = y;
            const int32_t z = (int32_t)((uint32_t)pt[2] - (uint32_t)head[2]);
            GP4_GLOBAL(int32_t, 0x00623e20) = z;
            rotateSample(c, GP4_ARRAY(uint8_t, 0x006232a8), x, y, z);
            gp4::call_regs(0x0043a148, c);
        }
        pt += 5;
        ++i;
    } while (i != GP4_FIELD(uint32_t, blk, 0x18));

    // A side never reached keeps its start value: no contact (popal; stc after the equal cmp).
    if (GP4_GLOBAL(int32_t, 0x00609808) == start || GP4_GLOBAL(int32_t, 0x0060980c) == start
        || GP4_GLOBAL(int32_t, 0x00609818) == start || GP4_GLOBAL(int32_t, 0x0060981c) == start) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x45u;
        return;
    }

    c.eax = (uint32_t)GP4_GLOBAL(int32_t, 0x00609808) - (uint32_t)GP4_GLOBAL(int32_t, 0x0060980c);
    c.edx = (uint32_t)GP4_GLOBAL(int32_t, 0x00609800) - (uint32_t)GP4_GLOBAL(int32_t, 0x00609804);
    gp4::call_regs(0x00401315, c);
    GP4_GLOBAL(uint32_t, 0x00609824) = clampAngle(c.eax);

    c.eax = (uint32_t)GP4_GLOBAL(int32_t, 0x00609818) - (uint32_t)GP4_GLOBAL(int32_t, 0x0060981c);
    c.edx = (uint32_t)GP4_GLOBAL(int32_t, 0x00609810) - (uint32_t)GP4_GLOBAL(int32_t, 0x00609814);
    gp4::call_regs(0x00401315, c);
    GP4_GLOBAL(uint32_t, 0x00609820) = clampAngle(c.eax);

    c.eax = GP4_GLOBAL(uint32_t, 0x00609824) << 16;
    c.ebx = GP4_GLOBAL(uint32_t, 0x00609820) << 16;
    c.ecx = 0;
    gp4::call_regs(0x0043a079, c);

    uint32_t a0 = c.eax;
    uint32_t a1 = c.ebx;
    uint32_t a2 = c.ecx;
    int32_t* rec = (int32_t*)(uintptr_t)GP4_GLOBAL(uint32_t, 0x0060982c);
    if (GP4_GLOBAL(uint8_t, 0x006097fc) == 0) {
        a0 += 0x80000000u;
        a1 = 0u - a1;
        a2 = 0u - a2;
    }
    rec[4] = (int32_t)a0;
    int32_t pitch = (int32_t)(uint32_t)((uint64_t)((int64_t)(int32_t)a1 * (int64_t)GP4_GLOBAL(int32_t, 0x00608020)) >> 14);
    if (pitch < GP4_GLOBAL(int32_t, 0x00608024)) pitch = GP4_GLOBAL(int32_t, 0x00608024);
    rec[3] = pitch;
    rec[5] = (int32_t)a2;

    const uint32_t s0 = sideAtZero(GP4_GLOBAL(int32_t, 0x00609800), GP4_GLOBAL(int32_t, 0x00609804),
                                   GP4_GLOBAL(int32_t, 0x00609808), GP4_GLOBAL(int32_t, 0x0060980c));
    const uint32_t s1 = sideAtZero(GP4_GLOBAL(int32_t, 0x00609810), GP4_GLOBAL(int32_t, 0x00609814),
                                   GP4_GLOBAL(int32_t, 0x00609818), GP4_GLOBAL(int32_t, 0x0060981c));
    GP4_GLOBAL(int32_t, 0x00608298) = (int32_t)(s1 + s0) >> 1;

    rotateCentre(c, GP4_ARRAY(uint8_t, 0x006231d8));

    rec = (int32_t*)(uintptr_t)GP4_GLOBAL(uint32_t, 0x0060982c);
    const int32_t* const pos = (const int32_t*)obj;
    rec[0] = (int32_t)((uint32_t)pos[0] + (uint32_t)GP4_GLOBAL(int32_t, 0x00623e30));
    rec[1] = (int32_t)((uint32_t)pos[1] + (uint32_t)GP4_GLOBAL(int32_t, 0x00623e34));
    const uint32_t lastLeft = (uint32_t)pos[2];
    const uint32_t lastRight = (uint32_t)GP4_GLOBAL(int32_t, 0x00623e38);
    rec[2] = (int32_t)(lastLeft + lastRight);

    rec[6] = pos[6];
    rec[7] = pos[7];
    rec[8] = pos[8];
    orientRecord(c, GP4_ARRAY(uint8_t, 0x00622f68), rec + 3, rec + 6);

    rec[9] = GP4_GLOBAL(int32_t, 0x00608298);
    rec[10] = pos[0x48 / 4];
    rec[11] = (int32_t)(uint32_t)GP4_FIELD(uint8_t, obj, 0x6d);
    rec[12] = pos[0x4c / 4];

    // popal keeps the flags of the last add (rec+8); clc clears CF.
    r->eflags = (r->eflags & ~0x8d5u) | (addFlags(lastLeft, lastRight) & ~1u);
}
GP4_IMPL(0x0043a32a, FUN_0043a32a, "regs(esi:ptr[0x78]:layout=obj_0043ac7f) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=shape_006099c0+bounds_0043a295+view_00609860+ang_006231d8")
