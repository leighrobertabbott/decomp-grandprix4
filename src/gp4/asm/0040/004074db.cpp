#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of SHL r32, count (count > 1) in the pinned oracle model: CF = last bit out, AF = 0,
// OF = bit31(value << (count - 1) ^ result).
static inline uint32_t shlFlags32(uint32_t value, uint32_t count) {
    const uint32_t result = value << count;
    const uint32_t previous = value << (count - 1);
    return (previous >> 31)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004074db
// edi = record. Rotates the offset (DAT_006278c0 - [edi+0xf0], DAT_006278c8 - [edi+0xf8]) by the angle
// [edi+0xe4] (scaled by DAT_006f2bdc) through the complex product 0x00401d41, rounds both parts to
// integers (DAT_006f2bf4 is the scratch dword) and derives a Q14 ratio:
//   eax = rounded x part, ecx = max(0x40000 - (x * word [edi+0x14] >> 14), 0x1000),
//   edx = (clamp(y - (x * word [edi+0x14] >> 15), +-0x400000) << 14) / ecx.
// ebx, esi and ebp are preserved. The flags are those of the "shl eax, 14" before the idiv
// (idiv leaves them untouched).
void __cdecl FUN_004074db(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const Ext angle = gp4::x87::ext_i32(GP4_FIELD(int32_t, record, 0xe4)) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc));
    Ext sine;
    Ext cosine;
    gp4::x87::fsincos(angle, &sine, &cosine);

    const int32_t offsetB = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006278c8) - GP4_FIELD(uint32_t, record, 0xf8));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = offsetB;
    const Ext partB = gp4::x87::ext_i32(offsetB);
    const int32_t offsetA = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006278c0) - GP4_FIELD(uint32_t, record, 0xf0));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = offsetA;
    const Ext partA = gp4::x87::ext_i32(offsetA);

    gp4::Regs product = *r;
    product.st_in = 0;
    product.st_out = 0;
    product.st_xin_mask = 0;
    product.st_xout_mask = 0;
    gp4::x87::set_in_st(product, 0, partA);
    gp4::x87::set_in_st(product, 1, partB);
    gp4::x87::set_in_st(product, 2, cosine);
    gp4::x87::set_in_st(product, 3, sine);
    product.st_in = 4;
    product.st_out = 2;
    gp4::call_regs(0x00401d41, product);

    const int32_t real = gp4::x87::fistp32(gp4::x87::out_st(product, 0));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = real;
    const int32_t imag = gp4::x87::fistp32(gp4::x87::out_st(product, 1));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = imag;

    const int64_t wide = static_cast<int64_t>(real) * static_cast<int64_t>(GP4_FIELD(int16_t, record, 0x14));
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(wide) >> 14);
    const int32_t half = static_cast<int32_t>(scaled) >> 1;
    int32_t divisor = static_cast<int32_t>(0x40000u - scaled);
    if (divisor < 0x1000) {
        divisor = 0x1000;
    }

    uint32_t numerator = static_cast<uint32_t>(imag) - static_cast<uint32_t>(half);
    if (static_cast<int32_t>(numerator) < 0) {
        numerator = 0u - numerator;
        if (!(numerator < 0x400000u)) {
            numerator = 0x400000u;
        }
        numerator = 0u - numerator;
    } else if (!(numerator < 0x400000u)) {
        numerator = 0x400000u;
    }

    int32_t remainder;
    const int64_t dividend = static_cast<int64_t>(static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(numerator))) << 14);
    const int32_t quotient = gp4::idiv64(dividend, divisor, &remainder);

    r->eax = static_cast<uint32_t>(real);
    r->ecx = static_cast<uint32_t>(divisor);
    r->edx = static_cast<uint32_t>(quotient);
    r->eflags = (r->eflags & ~0x8d5u) | shlFlags32(numerator, 14);
}
GP4_IMPL(0x004074db, FUN_004074db, "regs(edi:ptr[0x104]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
