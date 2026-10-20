#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

// atan2(y, x) of two integers (FILD y, FILD x, FPATAN), scaled by the double at 0x6f29d4 and
// rounded by FISTP into the scratch dword 0x6f2bf4 (which also stages both FILD operands).
static inline int32_t scaledAngle(int32_t y, int32_t x) {
    using namespace gp4::x87;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = y;
    const Ext ey = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = x;
    const Ext ex = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    const Ext a = fpatan(ey, ex) * ext(GP4_GLOBAL(double, 0x006f29d4));
    const int32_t v = fistp32(a);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = v;
    return v;
}

// ((word a - word b) << 8 + dword c - dword d) >> 8, with the word difference wrapped to 16 bits.
static inline int32_t fixedDiff(uint8_t* p, uint8_t* q, int wordOff, int dwordOff) {
    const int16_t w = static_cast<int16_t>(static_cast<uint16_t>(GP4_FIELD(uint16_t, p, wordOff) -
                                                                 GP4_FIELD(uint16_t, q, wordOff)));
    uint32_t v = static_cast<uint32_t>(static_cast<int32_t>(w)) << 8;
    v += GP4_FIELD(uint32_t, p, dwordOff);
    v -= GP4_FIELD(uint32_t, q, dwordOff);
    return static_cast<int32_t>(v) >> 8;
}

// FUNCTION: GP4 0x00555941
// ebx and edi are records (the caller walks the 0x160-stride array at 0x8943a0 with ebx one record
// ahead of edi). Four angles from edi to ebx go into the words edi+0x128/+0x12a/+0x12c/+0x12e:
// atan2 of the (word<<8 + dword) differences +0xd4/+0xf0 over +0xdc/+0xf8 and +0xd6/+0xf0 over
// +0xde/+0xf8, then of the dword differences +0x118 over +0x11c and +0x120 over +0x124. After the
// first two, the dwords +0x104 / +0x110 are pulled to angle<<16 -/+ 0x40000 when they are within
// 0x20000000 of angle<<16 and on the near side. Outputs: eax = last angle, edx = last difference,
// ebp = |second offset|, flags of the last `sub`.
void __cdecl FUN_00555941(gp4::Regs* r) {
    uint8_t* const next = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    // First angle and the +0x104 adjustment.
    int32_t y = fixedDiff(next, rec, 0xd4, 0xf0);
    int32_t x = fixedDiff(next, rec, 0xdc, 0xf8);
    uint32_t angle = static_cast<uint32_t>(scaledAngle(y, x));
    GP4_FIELD(uint16_t, rec, 0x128) = static_cast<uint16_t>(angle);
    uint32_t shifted = angle << 16;
    int32_t offset = static_cast<int32_t>(GP4_FIELD(uint32_t, rec, 0x104) - shifted);
    uint32_t magnitude = static_cast<uint32_t>(offset);
    if (offset < 0) magnitude = 0u - magnitude;
    if (static_cast<int32_t>(magnitude) < 0x20000000 && offset > -0x40000) {
        GP4_FIELD(uint32_t, rec, 0x104) = shifted - 0x40000u;
    }

    // Second angle and the +0x110 adjustment.
    y = fixedDiff(next, rec, 0xd6, 0xf0);
    x = fixedDiff(next, rec, 0xde, 0xf8);
    angle = static_cast<uint32_t>(scaledAngle(y, x));
    GP4_FIELD(uint16_t, rec, 0x12a) = static_cast<uint16_t>(angle);
    shifted = angle << 16;
    offset = static_cast<int32_t>(GP4_FIELD(uint32_t, rec, 0x110) - shifted);
    magnitude = static_cast<uint32_t>(offset);
    if (offset < 0) magnitude = 0u - magnitude;
    if (static_cast<int32_t>(magnitude) < 0x20000000 && offset < 0x40000) {
        GP4_FIELD(uint32_t, rec, 0x110) = shifted + 0x40000u;
    }
    r->ebp = magnitude;

    // Third and fourth angles from plain dword differences.
    y = static_cast<int32_t>(GP4_FIELD(uint32_t, next, 0x118) - GP4_FIELD(uint32_t, rec, 0x118));
    x = static_cast<int32_t>(GP4_FIELD(uint32_t, next, 0x11c) - GP4_FIELD(uint32_t, rec, 0x11c));
    angle = static_cast<uint32_t>(scaledAngle(y, x));
    GP4_FIELD(uint16_t, rec, 0x12c) = static_cast<uint16_t>(angle);

    const uint32_t a120 = GP4_FIELD(uint32_t, next, 0x120);
    const uint32_t b120 = GP4_FIELD(uint32_t, rec, 0x120);
    const uint32_t a124 = GP4_FIELD(uint32_t, next, 0x124);
    const uint32_t b124 = GP4_FIELD(uint32_t, rec, 0x124);
    angle = static_cast<uint32_t>(scaledAngle(static_cast<int32_t>(a120 - b120),
                                              static_cast<int32_t>(a124 - b124)));
    GP4_FIELD(uint16_t, rec, 0x12e) = static_cast<uint16_t>(angle);

    r->eax = angle;
    r->edx = a124 - b124;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(a124, b124));
}
GP4_IMPL(0x00555941, FUN_00555941, "regs(ebx:ptr[0x130]:bytes, edi:ptr[0x138]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of)")
