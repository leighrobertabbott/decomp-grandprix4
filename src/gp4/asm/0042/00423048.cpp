#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `shrd eax, edx, 14` on the 64-bit product edx:eax in the oracle model:
// CF = last bit shifted out, AF = 0, OF = bit 31 of (product >> 13) ^ result.
static inline uint32_t shrd14Flags(uint64_t product) {
    const uint32_t result = static_cast<uint32_t>(product >> 14);
    const uint32_t previous = static_cast<uint32_t>(product >> 13);
    return static_cast<uint32_t>((product >> 13) & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) & 0x80000000u) != 0u) ? 0x800u : 0u);
}

// (a * b) >> 14, the low dword ("imul; shrd eax, edx, 14").
static inline int32_t mulShift14(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 14);
}

// FUNCTION: GP4 0x00423048
// Force components from the velocity vector at edi (i32 +0x0c, +0x10, +0x14, limit +0x1c, scale +0x18):
//   [0x6080f4] = 0x4000, or (|v10 >> 5| * [0x6080ec] >> 14) + 0x4000 clamped to [0x6080f0] when
//   v10 >> 5 is negative; the heading is the angle (0x00401315) of (-v0c, -v14) and the planar speed
//   [0x6f2bf8] = round(sqrt(v0c^2 + v14^2)). A magnitude from the speed (>> 5, * [0x608008] >> 14,
//   capped at [0x60800c], * [0x609754] >> 14, * v18 >> 14) is scaled by min(v1c, round([0x6080f4] *
//   [0x608088] * [0x6f2b9c]) = [0x6f2bf4]) >> 14 and split by the Q14 cosine table (0x7c5c5c) of the
//   heading into [0x623e24] (the sine part, ebp) and [0x623e2c] (the cosine part, ecx restored).
//   eax = [0x623e2c], edx = the high half of the last product, ebp = the sine word; the flags are
//   those of the last shrd.
void __cdecl FUN_00423048(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_GLOBAL(int32_t, 0x006080f4) = 0x4000;
    const int32_t lift = GP4_FIELD(int32_t, body, 0x10) >> 5;
    if (lift < 0) {
        int32_t blend = mulShift14(-lift, GP4_GLOBAL(int32_t, 0x006080ec)) + 0x4000;
        if (blend > GP4_GLOBAL(int32_t, 0x006080f0)) {
            blend = GP4_GLOBAL(int32_t, 0x006080f0);
        }
        GP4_GLOBAL(int32_t, 0x006080f4) = blend;
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = 0u - GP4_FIELD(uint32_t, body, 0x0c);
    call.edx = 0u - GP4_FIELD(uint32_t, body, 0x14);
    gp4::call_regs(0x00401315, call);
    const uint16_t heading = static_cast<uint16_t>(call.eax);

    // sine word: table[|0x4000 - heading|], cosine word: table[|heading|] (16-bit negate, >> 2, even byte offset)
    uint16_t complement = static_cast<uint16_t>(0x4000u - heading);
    if (static_cast<int16_t>(complement) < 0) {
        complement = static_cast<uint16_t>(0u - complement);
    }
    const int32_t sine = GP4_ARRAY(int16_t, 0x007c5c5c)[complement >> 3];
    uint16_t magnitude = heading;
    if (static_cast<int16_t>(magnitude) < 0) {
        magnitude = static_cast<uint16_t>(0u - magnitude);
    }
    const int32_t cosine = GP4_ARRAY(int16_t, 0x007c5c5c)[magnitude >> 3];

    gp4::x87::Ext x = gp4::x87::ext_i32(GP4_FIELD(int32_t, body, 0x0c));
    x = x * x;
    gp4::x87::Ext z = gp4::x87::ext_i32(GP4_FIELD(int32_t, body, 0x14));
    z = z * z;
    GP4_GLOBAL(int32_t, 0x006f2bf8) = gp4::x87::fistp32(gp4::x87::fsqrt(x + z));

    int32_t force = GP4_GLOBAL(int32_t, 0x006f2bf8) >> 5;
    force = mulShift14(force, GP4_GLOBAL(int32_t, 0x00608008));
    if (force > GP4_GLOBAL(int32_t, 0x0060800c)) {
        force = GP4_GLOBAL(int32_t, 0x0060800c);
    }
    force = mulShift14(force, GP4_GLOBAL(int32_t, 0x00609754));
    force = mulShift14(force, GP4_FIELD(int32_t, body, 0x18));

    gp4::x87::Ext cap = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006080f4)) * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00608088));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = gp4::x87::fistp32(cap * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b9c)));
    int32_t limit = GP4_FIELD(int32_t, body, 0x1c);
    if (limit > GP4_GLOBAL(int32_t, 0x006f2bf4)) {
        limit = GP4_GLOBAL(int32_t, 0x006f2bf4);
    }

    force = mulShift14(force, limit);
    GP4_GLOBAL(int32_t, 0x00623e24) = mulShift14(force, sine);
    const int64_t last = static_cast<int64_t>(force) * static_cast<int64_t>(cosine);
    const int32_t across = static_cast<int32_t>(static_cast<uint64_t>(last) >> 14);
    GP4_GLOBAL(int32_t, 0x00623e2c) = across;

    r->eax = static_cast<uint32_t>(across);
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(last) >> 32);
    r->ebp = static_cast<uint32_t>(sine);
    r->eflags = (r->eflags & ~0x8d5u) | shrd14Flags(static_cast<uint64_t>(last));
}
GP4_IMPL(0x00423048, FUN_00423048, "regs(edi:ptr[0x40]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
