#include <gp4/gp4.h>

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Only the fields this routine reads.
struct Unknown_Object {
    uint8_t Unknown00[4];
    int16_t Unknown04;
    uint8_t Unknown06[2];
    int16_t Unknown08;
    uint8_t Unknown0A[0xb8 - 0x0a];
    int16_t UnknownB8;
    int16_t UnknownBA;
    uint8_t UnknownBC[4];
    int16_t UnknownC0;
    int16_t UnknownC2;
    uint8_t UnknownC4[0xd0 - 0xc4];
    uint16_t UnknownD0;
    uint16_t UnknownD2;
};
static_assert(sizeof(Unknown_Object) == 0xd4, "object size");

// movsx eax, word [0x7c5c5c + ((angle >> 2) & 0xfffe)]
static inline int32_t tableEntry(uint16_t angle) {
    const uint32_t byteOffset = static_cast<uint32_t>(angle >> 2) & 0xfffeu;
    return GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), byteOffset);
}

// imul + "shrd eax, edx, 14": bits 14..45 of the signed 64-bit product
static inline uint32_t q14(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 14);
}

// FUNCTION: GP4 0x00405ad0
// edi = object; ecx / ebx = a 2D point (in 16-bit fixed-point units); [0x603c78] selects the
// +0xb8.. or +0xba.. set of fields (position offsets >> 3 added to +4 / +8, and an angle).
// Result eax = rotated (point - position) projected on the angle's axis (Q14 table lookups),
// CF = sign of the result. ecx and ebx are saved and restored; edx = high half of the last product.
void __cdecl FUN_00405ad0(gp4::Regs* r) {
    const Unknown_Object* const obj = reinterpret_cast<const Unknown_Object*>(r->edi);
    const uint32_t pointA = r->ecx;
    const uint32_t pointB = r->ebx;

    uint32_t deltaA;
    uint32_t deltaB;
    uint16_t angle;
    if (GP4_GLOBAL(uint8_t, 0x00603c78) != 0) {
        deltaA = pointA - static_cast<uint32_t>((obj->UnknownB8 >> 3) + obj->Unknown04);
        deltaB = pointB - static_cast<uint32_t>((obj->UnknownC0 >> 3) + obj->Unknown08);
        angle = obj->UnknownD0;
    } else {
        deltaA = pointA - static_cast<uint32_t>((obj->UnknownBA >> 3) + obj->Unknown04);
        deltaB = pointB - static_cast<uint32_t>((obj->UnknownC2 >> 3) + obj->Unknown08);
        angle = obj->UnknownD2;
    }

    uint16_t complement = static_cast<uint16_t>(0x4000 - angle);
    if ((complement & 0x8000u) != 0) {
        complement = static_cast<uint16_t>(-complement);
    }
    const uint32_t first = q14(tableEntry(complement), static_cast<int32_t>(deltaA));

    uint16_t magnitude = angle;
    if ((magnitude & 0x8000u) != 0) {
        magnitude = static_cast<uint16_t>(-magnitude);
    }
    const int64_t product = static_cast<int64_t>(tableEntry(magnitude)) * static_cast<int64_t>(static_cast<int32_t>(deltaB));
    const uint32_t second = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t sum = second + first;

    uint32_t flags = additionFlags32(second, first);
    flags = (flags & ~1u) | ((flags & 0x80u) != 0u ? 1u : 0u);   // jns / stc / clc

    r->eax = sum;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00405ad0, FUN_00405ad0, "regs(ecx:i32, ebx:i32, edi:ptr[0xe0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
