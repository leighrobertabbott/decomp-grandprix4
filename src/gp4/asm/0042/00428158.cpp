#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit `left - right` (sub ax, word ptr [...]).
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return parityFlag(result)
        | (left < right ? 0x01u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Q14 cosine word table (16384 = 1.0) indexed by (|angle| >> 2) & 0xfffe bytes.
static inline int32_t cosineOf(int16_t angle) {
    uint16_t magnitude = static_cast<uint16_t>(angle);
    if ((magnitude & 0x8000u) != 0u) magnitude = static_cast<uint16_t>(0u - magnitude);
    const uint32_t index = (magnitude >> 2) & 0xfffeu;
    return static_cast<int32_t>(*reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(0x007c5c5cu + index)));
}

static inline uint32_t mulShift14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// Offset of a car's wheel point from the reference point in esi: with the word angle at edi+0xa, the
// radius (word +0x62 - 0x100, at least [0x624820]) and the offsets words +4 and +8 (<< 3),
//   [0x623e30] = (radius * cos(|a|)       >> 14) + word4 * 8 - ([esi]   >> 8)
//   [0x623e38] = word8 * 8 - (radius * cos(0x4000 - |a|) >> 14) - ([esi+4] >> 8)
// returns the angle of that vector (0x00401315) minus the word at esi+0x18, sign-extended; ecx = cos(|a|),
// ebp = cos(0x4000 - a) keep the two cosines, edx the second coordinate.
// FUNCTION: GP4 0x00428158
void __cdecl FUN_00428158(gp4::Regs* r) {
    const uint8_t* block = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint8_t* reference = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    const int16_t angle = *reinterpret_cast<const int16_t*>(block + 0xa);
    const int32_t cosComplement = cosineOf(static_cast<int16_t>(static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(angle))));
    const int32_t cosAngle = cosineOf(angle);

    int32_t radius = static_cast<int32_t>(*reinterpret_cast<const uint16_t*>(block + 0x62)) - 0x100;
    const int32_t floorRadius = GP4_GLOBAL(int32_t, 0x00624820);
    if (radius < floorRadius) radius = floorRadius;

    const uint32_t xRaw = mulShift14(radius, cosAngle)
        + (static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(block + 4))) << 3);
    GP4_GLOBAL(uint32_t, 0x00623e30) = xRaw;

    const uint32_t zRaw = (static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(block + 8))) << 3)
        - mulShift14(radius, cosComplement);
    GP4_GLOBAL(uint32_t, 0x00623e38) = zRaw;

    const int32_t refX = *reinterpret_cast<const int32_t*>(reference) >> 8;
    const uint32_t x = (0u - static_cast<uint32_t>(refX)) + GP4_GLOBAL(uint32_t, 0x00623e30);
    GP4_GLOBAL(uint32_t, 0x00623e30) = x;
    const int32_t refZ = *reinterpret_cast<const int32_t*>(reference + 4) >> 8;
    const uint32_t z = (0u - static_cast<uint32_t>(refZ)) + GP4_GLOBAL(uint32_t, 0x00623e38);
    GP4_GLOBAL(uint32_t, 0x00623e38) = z;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = x;
    call.edx = z;
    call.ebp = static_cast<uint32_t>(cosComplement);
    call.ecx = static_cast<uint32_t>(cosAngle);
    gp4::call_regs(0x00401315, call);

    const uint16_t heading = static_cast<uint16_t>(call.eax);
    const uint16_t reference18 = *reinterpret_cast<const uint16_t*>(reference + 0x18);
    const uint16_t diff = static_cast<uint16_t>(heading - reference18);

    r->eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(diff)));
    r->ecx = static_cast<uint32_t>(cosAngle);
    r->edx = z;
    r->ebp = static_cast<uint32_t>(cosComplement);
    r->eflags = (r->eflags & ~0x8d5u) | subFlags16(heading, reference18);
}
GP4_IMPL(0x00428158, FUN_00428158, "regs(esi:ptr[0x20]:bytes, edi:ptr[0x6a]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
