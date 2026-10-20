#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a logic result (or / test): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result, uint32_t signBit) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & signBit) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `left - right` (cmp).
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (left < right ? 0x01u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of `shrd eax, edx, 14` (pinned oracle model: CF = last bit shifted out, AF = 0,
// OF = bit31(previous ^ result) with previous = the 64-bit value shifted by 13).
static inline uint32_t shrd14Flags(uint32_t low, uint32_t high) {
    const uint64_t concat = (static_cast<uint64_t>(high) << 32) | low;
    const uint32_t previous = static_cast<uint32_t>(concat >> 13);
    const uint32_t result = static_cast<uint32_t>(concat >> 14);
    return ((low >> 13) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00426147_Body {
    uint8_t Unknown00[0x66];
    int16_t Unknown66;
    uint8_t Unknown68[0x09];
    uint8_t Flags71;
    uint8_t Unknown72[0x1a0 - 0x72];
    int32_t Unknown1A0;
};

struct Unknown_00426147_Other {
    uint8_t Unknown00[0xa9];
    int8_t UnknownA9;
};

// A triangular weight of the position Unknown1A0 (0 .. [0x603e18]) in the range 0..0x4000, peaking at
// 0x2000, times (+-UnknownA9 << 14) - word +0x66 times [0x601b58], both in Q14. Returns 0 when the position
// is out of range, the signed byte at edi+0xa9 is 0 or its sign disagrees with bit 3 of +0x71.
// FUNCTION: GP4 0x00426147
void __cdecl FUN_00426147(gp4::Regs* r) {
    const Unknown_00426147_Body* body = reinterpret_cast<const Unknown_00426147_Body*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00426147_Other* other = reinterpret_cast<const Unknown_00426147_Other*>(static_cast<uintptr_t>(r->edi));

    const int32_t position = body->Unknown1A0;
    uint32_t flags;
    bool inRange = false;
    int32_t direction = 0;

    if (position < 0) {
        flags = logicFlags(static_cast<uint32_t>(position), 0x80000000u);
    } else {
        const int32_t limit = GP4_GLOBAL(int32_t, 0x00603e18);
        if (position >= limit) {
            flags = cmpFlags(static_cast<uint32_t>(position), static_cast<uint32_t>(limit));
        } else {
            direction = static_cast<int32_t>(other->UnknownA9);
            if (direction == 0) {
                flags = logicFlags(0u, 0x80000000u);
            } else if (direction > 0) {
                const uint32_t bit = body->Flags71 & 8u;
                flags = logicFlags(bit, 0x80u);
                inRange = (bit == 0u);
            } else {
                const uint32_t bit = body->Flags71 & 8u;
                flags = logicFlags(bit, 0x80u);
                if (bit != 0u) {
                    direction = -direction;
                    inRange = true;
                }
            }
        }
    }

    if (!inRange) {
        r->eax = 0;
        r->edx = static_cast<uint32_t>(position);
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    const int32_t value = static_cast<int32_t>(static_cast<uint32_t>(direction) << 14) - static_cast<int32_t>(body->Unknown66);
    int32_t distance = position - 0x2000;
    if (distance < 0) distance = -distance;
    int32_t weight = static_cast<int32_t>(0u - (static_cast<uint32_t>(distance) << 1)) + 0x4000;
    if (weight < 0) weight = 0;
    if (weight > 0x4000) weight = 0x4000;

    const int64_t first = static_cast<int64_t>(value) * static_cast<int64_t>(weight);
    const int32_t scaled = static_cast<int32_t>(static_cast<uint64_t>(first) >> 14);
    const int64_t second = static_cast<int64_t>(scaled) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601b58));
    const uint32_t low = static_cast<uint32_t>(second);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);

    r->eax = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14);
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | shrd14Flags(low, high);
}
GP4_IMPL(0x00426147, FUN_00426147, "regs(esi:ptr[0x1a8]:layout=car_00426147, edi:ptr[0xb0]:layout=seg_00426147) -> (eax, edx, cf, pf, af, zf, sf, of); cases=400; globals=fuzz:i32+layout=lim_00426147")
