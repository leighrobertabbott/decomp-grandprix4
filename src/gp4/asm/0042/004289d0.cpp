#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic result (and / or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint8_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
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

struct Unknown_004289d0 {
    uint8_t Unknown00[0x179];
    uint8_t Flags179;
    uint8_t Unknown17A[0x0a];
    int16_t Unknown184;
    uint8_t Unknown186[0x284 - 0x186];
    int32_t Unknown284;
};

// Hysteresis on a car record: |word +0x184| is compared with +0x284 scaled by 0x603854 (>> 24) and by
// 0x603bd8 (bit 7 of +0x179 set) or 0x603bd4 (clear) (>> 14); bit 7 of +0x179 follows the result and
// bit 3 takes the sign of the word when the limit is passed.
// FUNCTION: GP4 0x004289d0
void __cdecl FUN_004289d0(gp4::Regs* r) {
    Unknown_004289d0* car = reinterpret_cast<Unknown_004289d0*>(static_cast<uintptr_t>(r->esi));

    const int64_t first = static_cast<int64_t>(car->Unknown284) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    const int32_t scaled = static_cast<int32_t>(static_cast<uint64_t>(first) >> 24);
    const bool wasSet = (car->Flags179 & 0x80u) != 0u;

    int64_t second;
    if (wasSet) {
        second = static_cast<int64_t>(scaled) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603bd8));
    } else {
        second = static_cast<int64_t>(scaled) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603bd4));
    }
    const int32_t limit = static_cast<int32_t>(static_cast<uint64_t>(second) >> 14);

    int32_t magnitude = static_cast<int32_t>(car->Unknown184);
    if (magnitude < 0) magnitude = -magnitude;
    r->eax = static_cast<uint32_t>(limit);
    r->edx = static_cast<uint32_t>(magnitude);

    uint32_t flags = 0;
    bool update = false;
    if (wasSet) {
        if (magnitude < limit) {
            car->Flags179 = static_cast<uint8_t>(car->Flags179 & 0x7fu);
            flags = logicFlags8(car->Flags179);
        } else {
            update = true;
        }
    } else {
        if (magnitude < limit) {
            flags = cmpFlags(static_cast<uint32_t>(magnitude), static_cast<uint32_t>(limit));
        } else {
            car->Flags179 = static_cast<uint8_t>(car->Flags179 | 0x80u);
            update = true;
        }
    }
    if (update) {
        if (car->Unknown184 < 0) {
            car->Flags179 = static_cast<uint8_t>(car->Flags179 | 0x08u);
        } else {
            car->Flags179 = static_cast<uint8_t>(car->Flags179 & 0xf7u);
        }
        flags = logicFlags8(car->Flags179);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004289d0, FUN_004289d0, "regs(esi:ptr[0x28c]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
