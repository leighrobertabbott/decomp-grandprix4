#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 16-bit `add left, right`.
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left + right) & 0xffffu;
    return (result < left ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// imul ; shrd eax, edx, 16: low dword of (a * b) >> 16.
static inline uint32_t mulShr16(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16);
}

// The record esi: the position pair it receives and the words derived from the rotated point.
struct Unknown_00426ff4_Body {
    int32_t Unknown00;
    int32_t Unknown04;
    uint8_t Unknown08[0x90 - 0x08];
    int16_t Unknown90;
    int16_t Unknown92;
    uint8_t Unknown94[0x96 - 0x94];
    uint16_t Unknown96;
    uint8_t Unknown98[0xac - 0x98];
    int32_t UnknownAC;
};

// The record edi handed to 0x0040460a; only its word +0x14 is read here.
struct Unknown_00426ff4_Link {
    uint8_t Unknown00[0x14];
    int16_t Unknown14;
};

// FUNCTION: GP4 0x00426ff4
// Advances the point (esi+0, esi+4) along a circular arc. V = ([esi+0xac] * [0x603854]) >> 16 is the
// turn angle (scaled by [0x6f2bcc]); [0x603d30] is the arc length D. For |V| >= 0x10000 the exact
// chord (ebp = (1 - cos t) * D/t, ecx = sin t * D/t) is used, otherwise the approximation
// ebp = round(t * D) >> 1, ecx = D. (ebp, ecx) is rotated by -word [0x603d6a] (0x00401d41), offset
// by [0x603d28]/[0x603d2c] and stored to [esi], [esi+4], [0x623e30], [0x623e38]. 0x0040460a
// (edi = record) then maps it into [0x934df0]/[0x934df4]; their >> 8 go to words +0x90 / +0x92 and
// +0x96 = +0x92 + (word [edi+0x14] * word +0x90 >> 15) (just +0x92 when that word is 0). The flags
// are those of the final 16-bit add.
void __cdecl FUN_00426ff4(gp4::Regs* r) {
    using namespace gp4::x87;

    Unknown_00426ff4_Body* body = reinterpret_cast<Unknown_00426ff4_Body*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00426ff4_Link* link = reinterpret_cast<const Unknown_00426ff4_Link*>(static_cast<uintptr_t>(r->edi));

    const uint32_t angleRaw = mulShr16(body->UnknownAC, GP4_GLOBAL(int32_t, 0x00603854));
    uint32_t magnitude = angleRaw;
    if (static_cast<int32_t>(magnitude) < 0) {
        magnitude = 0u - magnitude;
    }

    GP4_GLOBAL(int32_t, 0x006f2bf4) = static_cast<int32_t>(angleRaw);
    const Ext theta = ext_i32(static_cast<int32_t>(angleRaw)) * ext(GP4_GLOBAL(double, 0x006f2bcc));

    int32_t ebp;
    int32_t ecx;
    if (static_cast<int32_t>(magnitude) < 0x10000) {
        const int32_t scaled = fistp32(theta * ext_i32(GP4_GLOBAL(int32_t, 0x00603d30)));
        GP4_GLOBAL(int32_t, 0x006f2bf4) = scaled;
        ebp = scaled >> 1;
        ecx = GP4_GLOBAL(int32_t, 0x00603d30);
    } else {
        const Ext ratio = ext_i32(GP4_GLOBAL(int32_t, 0x00603d30)) / theta;
        Ext sine;
        Ext cosine;
        fsincos(theta, &sine, &cosine);
        const Ext versine = ext_one() + fchs(cosine);
        ebp = fistp32(versine * ratio);
        GP4_GLOBAL(int32_t, 0x006f2bf4) = ebp;
        ecx = fistp32(ratio * sine);
        GP4_GLOBAL(int32_t, 0x006f2bf4) = ecx;
    }

    const int32_t heading = 0 - static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00603d6a));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = heading;
    const Ext turn = ext_i32(heading) * ext(GP4_GLOBAL(double, 0x006f2bc4));
    Ext turnSine;
    Ext turnCosine;
    fsincos(turn, &turnSine, &turnCosine);

    GP4_GLOBAL(int32_t, 0x006f2bf4) = ecx;
    const Ext partB = ext_i32(ecx);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = ebp;
    const Ext partA = ext_i32(ebp);

    gp4::Regs product = *r;
    product.st_in = 0;
    product.st_out = 0;
    product.st_xin_mask = 0;
    product.st_xout_mask = 0;
    set_in_st(product, 0, partA);
    set_in_st(product, 1, partB);
    set_in_st(product, 2, turnCosine);
    set_in_st(product, 3, turnSine);
    product.st_in = 4;
    product.st_out = 2;
    gp4::call_regs(0x00401d41, product);

    const int32_t real = fistp32(out_st(product, 0));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = real;
    const int32_t imag = fistp32(out_st(product, 1));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = imag;

    const uint32_t newX = static_cast<uint32_t>(real) + GP4_GLOBAL(uint32_t, 0x00603d28);
    body->Unknown00 = static_cast<int32_t>(newX);
    GP4_GLOBAL(uint32_t, 0x00623e30) = newX;
    const uint32_t newY = static_cast<uint32_t>(imag) + GP4_GLOBAL(uint32_t, 0x00603d2c);
    body->Unknown04 = static_cast<int32_t>(newY);
    GP4_GLOBAL(uint32_t, 0x00623e38) = newY;

    gp4::Regs mapped = *r;
    mapped.st_in = 0;
    mapped.st_out = 0;
    mapped.st_xin_mask = 0;
    mapped.st_xout_mask = 0;
    mapped.edi = r->edi;
    gp4::call_regs(0x0040460a, mapped);
    uint32_t edx = mapped.edx;

    body->Unknown90 = static_cast<int16_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00934df0)) >> 8);
    const int32_t yShifted = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00934df4)) >> 8;
    body->Unknown92 = static_cast<int16_t>(yShifted);

    uint32_t eax = static_cast<uint32_t>(yShifted);
    uint16_t ax = static_cast<uint16_t>(link->Unknown14);
    if (ax != 0u) {
        const uint32_t wide = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(ax)) * static_cast<int32_t>(body->Unknown90));
        ax = static_cast<uint16_t>(wide >> 15);
        edx = (edx & 0xffff0000u) | ax;
    }
    const uint16_t sum = static_cast<uint16_t>(ax + static_cast<uint16_t>(body->Unknown92));
    const uint32_t flags = addFlags16(ax, static_cast<uint16_t>(body->Unknown92));
    body->Unknown96 = sum;
    eax = (eax & 0xffff0000u) | sum;

    r->eax = eax;
    r->ecx = static_cast<uint32_t>(ecx);
    r->edx = edx;
    r->ebp = static_cast<uint32_t>(ebp);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426ff4, FUN_00426ff4, "regs(esi:ptr[0xb4]:i16[-3..3], edi:ptr[0x20]:i16[-3..3]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00405489")
