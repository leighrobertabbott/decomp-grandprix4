#include <gp4/gp4.h>

// esi: a car record (fields read here; the rest stays opaque).
struct Unknown_00402db6_Car {
    uint8_t Unknown00[0xe];
    uint16_t Unknown0E;     // heading, 0x8000 = pi
    uint8_t Unknown10[0x56];
    int16_t Unknown66;      // Q14 blend between the segment at edi and the next one
    uint8_t Unknown68[0x28];
    int16_t Unknown90;
    uint8_t Unknown92[0x1e];
    int16_t UnknownB0;
    uint8_t UnknownB2[0x1c6];
    int16_t Unknown278;
    uint8_t Unknown27A[4];
    int16_t Unknown27E;
};

// edi: a track-segment record; the next record follows at +0x160.
struct Unknown_00402db6_Segment {
    uint16_t Unknown00;     // angle, 0x8000 = pi
    int16_t Unknown02;
    uint8_t Unknown04[2];
    int16_t Unknown06;
    uint8_t Unknown08[0x1e];
    int16_t Unknown26;
    uint8_t Unknown28[4];
    uint8_t Flags2C;
    uint8_t Unknown2D[0x133];
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((((left ^ result) & (right ^ result)) >> 31) << 11);
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

// imul; shrd eax, edx, 6
static inline uint32_t mulShr6(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * b;
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 6);
}

// FUNCTION: GP4 0x00402db6
// Interpolates the segment at edi and the next one (+0x160) by the car's Q14 blend word +0x66 and
// returns: edx = height-like value (lerp of +6 << 8, + 0x7200, + (+0x278 + +0x27e) * 32, minus the
// [0x6f2bec]-scaled +0x26/+0x186 lerp times min(|+0x90|, +0xb0 + 0x93) >> 6 with the sign of +0x90,
// plus 0x00403497's edx past that limit when +0x2c bit 3 / bit 2 allows), ecx = lerp of +2/+0x162,
// and the car's (+0x26/+0x186 lerp, ecx) pair rotated by heading - lerped angle (cos/sin stored at
// 0x625a9c/0x625aa0): ebp = cos*a + sin*b, eax = cos*b - sin*a (+ 0x00401315 of
// ((+0x278 - +0x27e) << 6, [0x601334]) when the two words differ). Byte 0x625aba is cleared.
// ebx, esi and edi are preserved; the flags are those of the final add (or of the equal compare).
void __cdecl FUN_00402db6(gp4::Regs* r) {
    const Unknown_00402db6_Car* car =
        reinterpret_cast<const Unknown_00402db6_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00402db6_Segment* seg =
        reinterpret_cast<const Unknown_00402db6_Segment*>(static_cast<uintptr_t>(r->edi));
    const Unknown_00402db6_Segment* next = seg + 1;
    const int32_t blend = car->Unknown66;

    const uint32_t base6 = static_cast<uint32_t>(static_cast<int32_t>(seg->Unknown06)) << 8;
    const uint32_t next6 = static_cast<uint32_t>(static_cast<int32_t>(next->Unknown06)) << 8;
    uint32_t height = mulQ14(static_cast<int32_t>(next6 - base6), blend) + base6 + 0x7200u;
    height += static_cast<uint32_t>(static_cast<int32_t>(car->Unknown278) + car->Unknown27E) << 5;

    const int32_t base26 = seg->Unknown26;
    uint32_t along = mulQ14(next->Unknown26 - base26, blend) + static_cast<uint32_t>(base26);
    if (along != 0u) {
        const uint32_t scaled = mulQ14(static_cast<int32_t>(along), GP4_GLOBAL(int32_t, 0x006f2bec));
        int32_t magnitude = car->Unknown90;
        if (magnitude < 0) {
            magnitude = -magnitude;
        }
        const int32_t limit = car->UnknownB0 + 0x93;
        if (magnitude > limit) {
            magnitude = limit;
        }
        uint32_t drop = mulShr6(static_cast<int32_t>(scaled), magnitude);
        if (car->Unknown90 < 0) {
            drop = 0u - drop;
        }
        height -= drop;
    }

    GP4_GLOBAL(uint8_t, 0x00625aba) = 0;
    const uint8_t mask = car->Unknown90 < 0 ? 8u : 4u;
    if ((seg->Flags2C & mask) != 0u) {
        uint16_t excess = static_cast<uint16_t>(car->Unknown90);
        if (static_cast<int16_t>(excess) < 0) {
            excess = static_cast<uint16_t>(0u - excess);
        }
        excess = static_cast<uint16_t>(excess - static_cast<uint16_t>(car->UnknownB0) - 0x93u);
        if (static_cast<int16_t>(excess) >= 0) {
            gp4::Regs s = *r;
            s.eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(excess)));
            s.ebx = height;
            gp4::call_regs(0x00403497, s);
            height += s.edx;
            along += s.eax;
        }
    }

    const int32_t base2 = seg->Unknown02;
    const uint32_t lateral = static_cast<uint32_t>(base2) + mulQ14(next->Unknown02 - base2, blend);
    const int32_t turn = static_cast<int16_t>(static_cast<uint16_t>(next->Unknown00 - seg->Unknown00));
    const uint32_t angle = mulQ14(turn, blend) + static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(seg->Unknown00)));
    const uint16_t relative = static_cast<uint16_t>(car->Unknown0E - static_cast<uint16_t>(angle));
    const int32_t cosv = cosQ14(relative);
    const int32_t sinv = cosQ14(static_cast<uint16_t>(0x4000u - relative));
    GP4_GLOBAL(int32_t, 0x00625a9c) = cosv;
    GP4_GLOBAL(int32_t, 0x00625aa0) = sinv;

    const uint32_t first = mulQ14(cosv, static_cast<int32_t>(along)) + mulQ14(sinv, static_cast<int32_t>(lateral));
    uint32_t second = mulQ14(cosv, static_cast<int32_t>(lateral)) - mulQ14(sinv, static_cast<int32_t>(along));

    const uint16_t difference = static_cast<uint16_t>(car->Unknown278 - car->Unknown27E);
    uint32_t flags;
    if (difference != 0u) {
        gp4::Regs s = *r;
        s.eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(difference))) << 6;
        s.edx = GP4_GLOBAL(uint32_t, 0x00601334);
        gp4::call_regs(0x00401315, s);
        flags = addFlags(s.eax, second);
        second += s.eax;
    } else {
        flags = 0x44u;   // sub dx, equal word: ZF, PF
    }

    r->eax = second;
    r->ecx = lateral;
    r->edx = height;
    r->ebp = first;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00402db6, FUN_00402db6, "regs(esi:ptr[0x286]:layout=car_00402db6, edi:ptr[0x18e]:layout=seg_00402db6) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tbl_00868598+gain_00403497")
