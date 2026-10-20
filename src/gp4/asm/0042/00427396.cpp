#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `add left, right` (CF PF AF ZF SF OF).
static inline uint32_t addFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left + right);
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00427396_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1C;
    uint8_t Unknown20[0x92 - 0x20];
    uint16_t Unknown92;
    uint8_t Unknown94[0x96 - 0x94];
    uint16_t Unknown96;
    int32_t Unknown98;
    uint8_t Unknown9C[0xde - 0x9c];
    uint16_t UnknownDE;
    uint8_t UnknownE0[0x146 - 0xe0];
    uint16_t Unknown146;
    uint8_t Unknown148[0x1b8 - 0x148];
    uint32_t Segment1B8;
};

struct Unknown_00427396_Segment {
    uint8_t Unknown00[0x14];
    int16_t Unknown14;
};

// Low dword of (a * b) >> shift, as `imul; shrd eax, edx, shift` leaves it in eax.
static inline uint32_t mulShift(int32_t a, int32_t b, int shift) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> shift);
}

// Byte offset into the Q14 cosine/sine word table 0x007c5c5c for a 16-bit angle: (|a| >> 2) & 0xfffe.
static inline uint32_t tableOffset(uint16_t angle) {
    uint16_t magnitude = angle;
    if (static_cast<int16_t>(magnitude) < 0) {
        magnitude = static_cast<uint16_t>(0u - magnitude);
    }
    return static_cast<uint32_t>(magnitude >> 2) & 0xfffeu;
}

// esi = car record, edi = its track-segment record (ring at 0x008943a0). The radius
// (([esi+0x98] - [0x00603d38]) / 2 + [0x00603d38]) * [0x00603854] >> 24 goes to [0x00603d30]
// (its high dword's low word to +0xde); the point at that radius in the direction of the angle
// word [0x00603d6a] around ([0x00603d28], [0x00603d2c]) goes to 0x00623e30 / 0x00623e38. The
// segment then advances (at most 4 records, with the lap/pit wrap) while the next record's side
// test 0x00405e19 is clear, stored into +0x1b8 and +0x1c; 0x0040460a maps the point into that
// segment's frame and words +0x92, +0x146 and +0x96 are derived from it and the segment word +0x14.
// FUNCTION: GP4 0x00427396
void __cdecl FUN_00427396(gp4::Regs* r) {
    Unknown_00427396_Car* car = reinterpret_cast<Unknown_00427396_Car*>(static_cast<uintptr_t>(r->esi));

    const uint32_t base = GP4_GLOBAL(uint32_t, 0x00603d38);
    const int32_t half = static_cast<int32_t>(static_cast<uint32_t>(car->Unknown98) - base) >> 1;
    const int32_t mid = static_cast<int32_t>(static_cast<uint32_t>(half) + base);
    const int64_t product = static_cast<int64_t>(mid) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    const uint32_t radius = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 24);
    GP4_GLOBAL(uint32_t, 0x00603d30) = radius;
    car->UnknownDE = static_cast<uint16_t>(static_cast<uint64_t>(product) >> 32);

    const int16_t* table = GP4_ARRAY(int16_t, 0x007c5c5c);
    const uint16_t angle = GP4_GLOBAL(uint16_t, 0x00603d6a);
    const int32_t cosine = table[tableOffset(angle) >> 1];
    GP4_GLOBAL(int32_t, 0x00934de8) = cosine;
    const int32_t sine = -static_cast<int32_t>(table[tableOffset(static_cast<uint16_t>(0x4000u - angle)) >> 1]);
    GP4_GLOBAL(int32_t, 0x00934dec) = sine;

    const uint32_t x = (0u - mulShift(static_cast<int32_t>(radius), sine, 14)) + GP4_GLOBAL(uint32_t, 0x00603d28);
    const uint32_t z = mulShift(static_cast<int32_t>(radius), cosine, 14) + GP4_GLOBAL(uint32_t, 0x00603d2c);
    GP4_GLOBAL(uint32_t, 0x00623e30) = x;
    GP4_GLOBAL(uint32_t, 0x00623e38) = z;
    r->eax = 0;
    r->edx = 0;
    r->ebx = x;
    r->ecx = z;

    uint32_t segment = r->edi;
    uint32_t steps = 4;
    do {
        uint32_t next = segment + 0x160u;
        if (next >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (next < GP4_GLOBAL(uint32_t, 0x00891500)) {
                next -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (next >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                next = GP4_GLOBAL(uint32_t, 0x00866f04);
            }
        }
        r->ecx = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623e30) >> 11);
        r->ebx = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623e38) >> 11);
        r->edi = next;
        gp4::call_regs(0x00405e19, *r);
        r->edi = segment;
        if ((r->eflags & gp4::CF) != 0u) {
            break;
        }
        segment += 0x160u;
        if (segment >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (segment < GP4_GLOBAL(uint32_t, 0x00891500)) {
                segment -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (segment >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                segment = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
        car->Segment1B8 = segment;
        car->Segment1C = segment;
        --steps;
    } while (steps != 0u);
    r->edi = segment;
    r->ebp = steps;

    gp4::call_regs(0x0040460a, *r);
    const int32_t lateral = GP4_GLOBAL(int32_t, 0x00934df4) >> 8;
    r->eax = static_cast<uint32_t>(lateral);
    car->Unknown92 = static_cast<uint16_t>(lateral);
    r->ebx = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00934df0) >> 8);
    const int32_t along = static_cast<int16_t>(r->ebx);

    const Unknown_00427396_Segment* record =
        reinterpret_cast<const Unknown_00427396_Segment*>(static_cast<uintptr_t>(segment));
    int32_t width = record->Unknown14;
    uint16_t value = static_cast<uint16_t>(width);
    if (width != 0) {
        const uint16_t high = static_cast<uint16_t>(static_cast<uint32_t>(width * along) >> 14);
        r->edx = (r->edx & 0xffff0000u) | high;
        value = high;
    }
    value = static_cast<uint16_t>(0x400u - value);
    if (static_cast<int16_t>(value) < 0x10) {
        value = 0x10;
    }
    car->Unknown146 = value;

    width = record->Unknown14;
    value = static_cast<uint16_t>(width);
    if (width != 0) {
        const uint16_t high = static_cast<uint16_t>(static_cast<uint32_t>(width * along) >> 15);
        r->edx = (r->edx & 0xffff0000u) | high;
        value = high;
    }
    const uint16_t offset = car->Unknown92;
    const uint16_t sum = static_cast<uint16_t>(value + offset);
    car->Unknown96 = sum;
    r->eax = (r->eax & 0xffff0000u) | sum;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags16(value, offset);
}
GP4_IMPL(0x00427396, FUN_00427396, "regs(edi:rec[0x8943a0,0x160,0..14], esi:ptr[0x1c0]:bytes) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_00405a12+links_00405a12")
