#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by "shrd eax, edx, 14" on the 64-bit product edx:eax (pinned oracle model).
static inline uint32_t shrd14Flags(uint64_t product, uint32_t result) {
    const uint32_t previous = static_cast<uint32_t>(product >> 13);
    return static_cast<uint32_t>((product >> 13) & 1u) | parityFlag(result)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) >> 31) << 11);
}

// FUNCTION: GP4 0x004169c3
// ebx = car body. Rotates the pair (+0x190, +0x194) by the angle +0x00 (2^31 = pi) into
// (+0x18, +0x1c), stores the angle's sine / cosine scaled by [0x6f2b04] at +0x78 / +0x7c, clamps
// |+0x1c| to at least [0x603498] (+0x2ac and [0x6034a4]), runs 0x00416981, then derives the
// vector length +0x2b0 = sqrt(+0x40^2 + +0x18^2), the ratio +0x2b4 (0x4000, or (length << 14) /
// [0x60349c] below it, kept in ebp) and the scaled values +0x30 (atan2) and +0x28 (0x00401ccc).
// eax / edx hold the last product (imul ebp; shrd 14); the flags are those of that shrd.
void __cdecl FUN_004169c3(gp4::Regs* r) {
    using namespace gp4::x87;

    uint8_t* const body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    GP4_GLOBAL(int32_t, 0x006f2bf4) = GP4_FIELD(int32_t, body, 0);
    const Ext angle = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)) * ext(GP4_GLOBAL(double, 0x006f2bdc));
    Ext sine;
    Ext cosine;
    fsincos(angle, &sine, &cosine);
    GP4_FIELD(int32_t, body, 0x7c) = fistp32(cosine * ext(GP4_GLOBAL(double, 0x006f2b04)));
    GP4_FIELD(int32_t, body, 0x78) = fistp32(sine * ext(GP4_GLOBAL(double, 0x006f2b04)));

    {
        // 0x401d41: (a + ib) * (cos + i sin) with a = +0x190, b = +0x194
        gp4::Regs call = *r;
        call.st_in = 4;
        call.st_out = 2;
        set_in_st(call, 0, ext_i32(GP4_FIELD(int32_t, body, 0x190)));
        set_in_st(call, 1, ext_i32(GP4_FIELD(int32_t, body, 0x194)));
        set_in_st(call, 2, cosine);
        set_in_st(call, 3, sine);
        gp4::call_regs(0x00401d41, call);
        GP4_FIELD(int32_t, body, 0x18) = fistp32(out_st(call, 0));
        GP4_FIELD(int32_t, body, 0x1c) = fistp32(out_st(call, 1));
    }

    int32_t clamp = GP4_FIELD(int32_t, body, 0x1c);
    if (clamp < 0) clamp = static_cast<int32_t>(0u - static_cast<uint32_t>(clamp));
    if (clamp < GP4_GLOBAL(int32_t, 0x00603498)) clamp = GP4_GLOBAL(int32_t, 0x00603498);
    GP4_FIELD(int32_t, body, 0x2ac) = clamp;
    GP4_GLOBAL(int32_t, 0x006034a4) = clamp;

    {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x00416981, call);
    }

    const Ext across = ext_i32(GP4_FIELD(int32_t, body, 0x40));
    const Ext along = ext_i32(GP4_FIELD(int32_t, body, 0x18));
    GP4_FIELD(int32_t, body, 0x2b0) = fistp32(fsqrt(across * across + along * along));

    int32_t ratio = 0x4000;
    const int32_t length = GP4_FIELD(int32_t, body, 0x2b0);
    if (length < GP4_GLOBAL(int32_t, 0x0060349c)) {
        int32_t remainder;
        ratio = gp4::idiv64(static_cast<int64_t>(length) << 14, GP4_GLOBAL(int32_t, 0x0060349c), &remainder);
    }
    GP4_FIELD(int32_t, body, 0x2b4) = ratio;

    int32_t depth = GP4_GLOBAL(int32_t, 0x006034a4);
    if (GP4_FIELD(int32_t, body, 0x1c) < 0) depth = static_cast<int32_t>(0u - static_cast<uint32_t>(depth));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = depth;
    const Ext heading = fpatan(ext_i32(GP4_FIELD(int32_t, body, 0x18)), ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(heading * ext(GP4_GLOBAL(double, 0x006f29d4)));
    const int64_t first = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006f2bf4)) * static_cast<int64_t>(ratio);
    GP4_FIELD(uint32_t, body, 0x30) = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);

    {
        // 0x401ccc: st0 = limit [0x6f2a8c], st1 = [0x6034a4], st2 = +0x40
        gp4::Regs call = *r;
        call.st_in = 3;
        call.st_out = 1;
        set_in_st(call, 0, ext(GP4_GLOBAL(double, 0x006f2a8c)));
        set_in_st(call, 1, ext_i32(GP4_GLOBAL(int32_t, 0x006034a4)));
        set_in_st(call, 2, ext_i32(GP4_FIELD(int32_t, body, 0x40)));
        gp4::call_regs(0x00401ccc, call);
        GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(out_st(call, 0) * ext(GP4_GLOBAL(double, 0x006f2b14)));
    }
    int32_t magnitude = GP4_GLOBAL(int32_t, 0x006f2bf4);
    if (magnitude < 0) magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(magnitude));
    const int64_t second = static_cast<int64_t>(magnitude) * static_cast<int64_t>(ratio);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14);
    GP4_FIELD(uint32_t, body, 0x28) = result;

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
    r->ebp = static_cast<uint32_t>(ratio);
    r->eflags = (r->eflags & ~0x8d5u) | shrd14Flags(static_cast<uint64_t>(second), result);
}
GP4_IMPL(0x004169c3, FUN_004169c3, "regs(ebx:ptr[0x2bc]:layout=body_004169c3) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=floor_00603498")
