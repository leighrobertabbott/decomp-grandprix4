#include <gp4/gp4.h>

// esi: a car record (0x41c-stride array at 0x0066fef0).
struct Unknown_004382d4_Car {
    int32_t Unknown00;      // x (track units << 11)
    int32_t Unknown04;      // z (track units << 11)
    uint8_t Unknown08[6];
    uint16_t Unknown0E;     // heading, 0x8000 = pi
    uint8_t Unknown10[0xc];
    uint32_t Unknown1C;     // current track-segment record (0x160-stride array at 0x008943a0)
};

// A track-segment record of the 0x160-stride array at 0x008943a0.
struct Unknown_004382d4_Segment {
    uint16_t Unknown00;     // angle, 0x8000 = pi
    uint16_t Unknown02;
    int16_t Unknown04;      // x (track units)
    uint16_t Unknown06;
    int16_t Unknown08;      // z (track units)
    uint8_t Unknown0A[0xa];
    int16_t Unknown14;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 0x04u : 0u;
}

// Q14 cosine of a 16-bit angle through the word table at 0x7c5c5c (|angle| >> 3 indexes it).
static inline int32_t cosQ14(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    return GP4_ARRAY(const int16_t, 0x007c5c5c)[angle >> 3];
}

// imul; shrd eax, edx, 14
static inline uint32_t mulQ14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * b;
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// imul; shrd eax, edx, 15
static inline uint32_t mulQ15(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * b;
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 15);
}

// FUNCTION: GP4 0x004382d4
// Projects a point at distance [0x629f24] from the car at esi in the direction heading +
// word [0x629f1c + 2*eax] (stored at 0x629ea4/0x629ea8, >> 11 at 0x623e30/0x623e38), finds its
// track segment from the car's one with 0x00406262 (stored at 0x629ec0) and expresses the point
// in that segment's frame: 0x934df0/0x629f14 = lateral, 0x934df4 = along (after the +0x14
// correction), 0x934de8/0x934dec = cos/sin of the segment angle, and 0x629f18 =
// clamp((along << 14) / max(0x400 - 2*corr, 0x40), 0, 0x4000).
// pushal/popal: every register is preserved; the flags are those of `cmp eax, 0` on the
// quotient after the upper clamp.
void __cdecl FUN_004382d4(gp4::Regs* r) {
    const uint32_t index = r->eax;
    const Unknown_004382d4_Car* car =
        reinterpret_cast<const Unknown_004382d4_Car*>(static_cast<uintptr_t>(r->esi));
    const int32_t radius = GP4_GLOBAL(int32_t, 0x00629f24);
    const uint16_t angle = static_cast<uint16_t>(GP4_ARRAY(uint16_t, 0x00629f1c)[index] + car->Unknown0E);

    const uint32_t x = mulQ14(cosQ14(static_cast<uint16_t>(0x4000u - angle)), radius)
        + static_cast<uint32_t>(car->Unknown00);
    GP4_GLOBAL(uint32_t, 0x00629ea4) = x;
    GP4_GLOBAL(int32_t, 0x00623e30) = static_cast<int32_t>(x) >> 11;
    const uint32_t z = mulQ14(cosQ14(angle), radius) + static_cast<uint32_t>(car->Unknown04);
    GP4_GLOBAL(uint32_t, 0x00629ea8) = z;
    GP4_GLOBAL(int32_t, 0x00623e38) = static_cast<int32_t>(z) >> 11;

    gp4::Regs s = *r;
    s.edi = car->Unknown1C;
    gp4::call_regs(0x00406262, s);
    GP4_GLOBAL(uint32_t, 0x00629ec0) = s.edx;
    const Unknown_004382d4_Segment* seg =
        reinterpret_cast<const Unknown_004382d4_Segment*>(static_cast<uintptr_t>(s.edx));

    const int32_t dx = static_cast<int32_t>(x - (static_cast<uint32_t>(static_cast<int32_t>(seg->Unknown04)) << 11));
    const int32_t dz = static_cast<int32_t>(z - (static_cast<uint32_t>(static_cast<int32_t>(seg->Unknown08)) << 11));
    const int32_t c = cosQ14(seg->Unknown00);
    const int32_t sn = cosQ14(static_cast<uint16_t>(0x4000u - seg->Unknown00));
    GP4_GLOBAL(int32_t, 0x00934de8) = c;
    GP4_GLOBAL(int32_t, 0x00934dec) = sn;

    const int32_t lateral = static_cast<int32_t>(mulQ14(dx, c) - mulQ14(dz, sn)) >> 8;
    int32_t along = static_cast<int32_t>(mulQ14(dz, c) + mulQ14(dx, sn)) >> 8;

    uint32_t correction = 0;
    const int32_t w = seg->Unknown14;
    if (w != 0) {
        correction = mulQ15(w, lateral);
        along = static_cast<int32_t>(static_cast<uint32_t>(along) - correction);
    }
    int32_t divisor = static_cast<int32_t>(0x400u - (correction << 1));
    if (divisor < 0x40) {
        divisor = 0x40;
    }
    int32_t q = static_cast<int32_t>(static_cast<uint32_t>(along) << 14) / divisor;
    if (q > 0x4000) {
        q = 0x4000;
    }
    const uint32_t qu = static_cast<uint32_t>(q);
    const uint32_t flags = parityFlag(qu) | (qu == 0u ? 0x40u : 0u) | ((qu >> 31) << 7);
    if (q < 0) {
        q = 0;
    }
    GP4_GLOBAL(int32_t, 0x00629f18) = q;
    GP4_GLOBAL(int32_t, 0x00629f14) = lateral;
    GP4_GLOBAL(int32_t, 0x00934df0) = lateral;
    GP4_GLOBAL(int32_t, 0x00934df4) = along;

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004382d4, FUN_004382d4, "regs(eax:u32[0..3], esi:u32[0x66fef0..0x66fef0]) -> (cf, pf, af, zf, sf, of); globals=layout=ring_004317ae+ring_0043edd3+car0_004382d4+ring_00405a12")
