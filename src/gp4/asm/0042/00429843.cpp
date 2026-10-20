#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of "neg value".
static inline uint32_t negationFlags32(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | ((value ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

struct Unknown_00429843_Esi {
    uint8_t Unknown00[0x0e];
    int16_t Unknown0E;
    uint8_t Unknown10[0x18 - 0x10];
    int16_t Unknown18;
};

// FUNCTION: GP4 0x00429843
// eax = atan2(DAT_00601334 * sin(a * k), |DAT_00601354 * cos(b * k) + (DAT_00601acc >> 1)|) * DAT_006f29d4,
// rounded, with a = the 16-bit sum DAT_00601b14 + (esi.word[0x0e] - esi.word[0x18]) and b = DAT_00601b14;
// the magnitude is clamped to the unsigned limit DAT_006013ac and the sign restored. edx = the
// sign-extended 16-bit word difference. The flags are those of the final cmp (positive) or neg (negative).
void __cdecl FUN_00429843(gp4::Regs* r) {
    const Unknown_00429843_Esi* record = reinterpret_cast<const Unknown_00429843_Esi*>(static_cast<uintptr_t>(r->esi));

    const int16_t difference = static_cast<int16_t>(static_cast<uint16_t>(record->Unknown0E) - static_cast<uint16_t>(record->Unknown18));
    const int32_t wordDifference = difference;
    const int32_t base = GP4_GLOBAL(int32_t, 0x00601b14);
    const int32_t shifted = static_cast<int16_t>(static_cast<uint16_t>(base + wordDifference));

    const gp4::x87::Ext sine = gp4::x87::fsin(
        gp4::x87::ext_i32(shifted) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f29cc)));
    const gp4::x87::Ext y = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00601334)) * sine;

    const gp4::x87::Ext cosine = gp4::x87::fcos(
        gp4::x87::ext_i32(base) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f29cc)));
    const gp4::x87::Ext scaledCosine = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00601354)) * cosine;
    const int32_t half = GP4_GLOBAL(int32_t, 0x00601acc) >> 1;
    const gp4::x87::Ext x = gp4::x87::fabs(scaledCosine + gp4::x87::ext_i32(half));

    const int32_t angle = gp4::x87::fistp32(gp4::x87::fpatan(y, x) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f29d4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = angle;

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006013ac);
    uint32_t result;
    uint32_t flags;
    if (angle < 0) {
        uint32_t magnitude = 0u - static_cast<uint32_t>(angle);
        if (magnitude >= limit) {
            magnitude = limit;
        }
        result = 0u - magnitude;
        flags = negationFlags32(magnitude);
    } else {
        result = static_cast<uint32_t>(angle);
        flags = subtractionFlags32(result, limit);
        if (result >= limit) {
            result = limit;
        }
    }

    r->eax = result;
    r->edx = static_cast<uint32_t>(wordDifference);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00429843, FUN_00429843, "regs(esi:ptr[0x40]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
