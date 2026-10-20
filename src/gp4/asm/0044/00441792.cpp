#include <gp4/gp4.h>

// esi: a car record (only the two bytes read here are named).
struct Unknown_00441792_Car {
    uint8_t Unknown00[0x73];
    uint8_t Unknown73;              // car number, 1-based
    uint8_t Unknown74[0x7c - 0x74];
    uint8_t Unknown7C;              // bit 7 set: the scaling is skipped
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `test byte, 0xff` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// One-operand `imul` then `shrd eax, edx, 14`: eax = bits 14..45 of the signed 64-bit product. Flags of the shrd
// (oracle model: CF = bit 13, AF = 0, OF = bit 31 of the shift-13 result xor the shift-14 result).
static inline uint32_t mulShift14(int32_t a, int32_t b, uint32_t* flags) {
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b));
    const uint32_t result = static_cast<uint32_t>(product >> 14);
    const uint32_t previous = static_cast<uint32_t>(product >> 13);
    *flags = ((static_cast<uint32_t>(product) >> 13) & 1u) | parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u) | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    return result;
}

// FUNCTION: GP4 0x00441792
// Piecewise-linear remap of eax (a distance in 0..0xa00): returned unchanged at 0xa00 and above, when the car's
// byte +0x7c has bit 7 set, when edx >= 2 (signed) or when the per-car flag byte [0x62ebf0 + car number] is
// clear. Otherwise the three segments use the Q14 factor f = [0x62ebfc]: eax * f below 0x280; above that
// (eax - 0x280) * (0x8000 - f) plus 0x280 * f up to 0x780; and (eax - 0x780) * f plus 0xa00 - 0x280 * f beyond.
// ebp and edx are saved and restored; the flags are those of the last instruction on the taken path.
void __cdecl FUN_00441792(gp4::Regs* r) {
    const Unknown_00441792_Car* car = reinterpret_cast<const Unknown_00441792_Car*>(static_cast<uintptr_t>(r->esi));
    const int32_t value = static_cast<int32_t>(r->eax);
    const int32_t selector = static_cast<int32_t>(r->edx);
    uint32_t result = r->eax;
    uint32_t flags;

    if (value >= 0xa00) {
        flags = cmpFlags32(r->eax, 0xa00u);
    } else if ((car->Unknown7C & 0x80u) != 0u) {
        flags = testFlags8(car->Unknown7C);
    } else if (selector >= 2) {
        flags = cmpFlags32(r->edx, 2u);
    } else {
        const uint32_t carNumber = car->Unknown73;
        const uint8_t enabled = GP4_ARRAY(const uint8_t, 0x0062ebf1)[carNumber - 1u];
        if (enabled == 0u) {
            flags = cmpFlags8(enabled, 0u);
        } else {
            const int32_t factor = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062ebfc));
            uint32_t shiftFlags;
            if (value < 0x780) {
                if (value < 0x280) {
                    result = mulShift14(value, factor, &shiftFlags);
                    flags = shiftFlags;
                } else {
                    const int32_t rest = static_cast<int32_t>(0x8000u - static_cast<uint32_t>(factor));
                    const uint32_t upper = mulShift14(value - 0x280, rest, &shiftFlags);
                    const uint32_t lower = mulShift14(0x280, factor, &shiftFlags);
                    result = lower + upper;
                    flags = addFlags32(lower, upper);
                }
            } else {
                const uint32_t upper = mulShift14(value - 0x780, factor, &shiftFlags);
                const uint32_t lower = mulShift14(0x280, factor, &shiftFlags);
                const uint32_t base = (0u - lower) + 0xa00u;
                result = base + upper;
                flags = addFlags32(base, upper);
            }
        }
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441792, FUN_00441792, "regs(eax:i32[-1000..3000], edx:i32[-3..3], esi:ptr[0x84]:layout=team_00441792) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=team_0062ebf1")
