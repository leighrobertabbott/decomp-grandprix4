#include <gp4/gp4.h>

struct UnknownRecord_0041a2ec {
    uint8_t Unknown00[0x5e];
    int16_t Unknown5E;
    uint8_t Unknown60[0x173 - 0x60];
    uint8_t Unknown173;
    uint8_t Unknown174[0x23c - 0x174];
    int16_t Unknown23C;
    uint8_t Unknown23E[0x2f4 - 0x23e];
    int32_t Unknown2F4;
    int32_t Unknown2F8;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left - right` whose result is `result` (cmp / neg).
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Sign-magnitude clamp: |value| capped at `limit` (unsigned compare, as the original does)
// with the sign restored.
static inline int32_t clampMagnitude(int32_t value, uint32_t limit) {
    if (value < 0) {
        uint32_t magnitude = 0u - static_cast<uint32_t>(value);
        if (magnitude >= limit) magnitude = limit;
        return static_cast<int32_t>(0u - magnitude);
    }
    uint32_t magnitude = static_cast<uint32_t>(value);
    if (magnitude >= limit) magnitude = limit;
    return static_cast<int32_t>(magnitude);
}

// FUNCTION: GP4 0x0041a2ec
void __cdecl FUN_0041a2ec(gp4::Regs* r) {
    UnknownRecord_0041a2ec* self = reinterpret_cast<UnknownRecord_0041a2ec*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(int32_t, 0x00603780) = 0x4a00 - GP4_GLOBAL(int32_t, 0x00603854);

    int32_t first = GP4_GLOBAL(int32_t, 0x00638ce0);
    if (first < 0) first = static_cast<int32_t>(0u - static_cast<uint32_t>(first));
    int32_t second = GP4_GLOBAL(int32_t, 0x00638cd8);
    if (second < 0) second = static_cast<int32_t>(0u - static_cast<uint32_t>(second));
    if (first < second) first = second;

    int32_t level = static_cast<int32_t>(static_cast<uint32_t>(first) >> 8);
    if (level > 0x4000) level = 0x4000;
    int32_t grip = static_cast<int32_t>(static_cast<uint32_t>(
        (static_cast<int64_t>(level) * GP4_GLOBAL(int32_t, 0x0060377c)) >> 14));
    if (grip > 0x4000) grip = 0x4000;
    const int32_t remaining = 0x4000 - grip;

    const int32_t difference = static_cast<int32_t>(self->Unknown23C) - GP4_GLOBAL(int32_t, 0x00638d20);
    const int32_t limited = clampMagnitude(difference, 0x4000u);
    int32_t remainder = 0;
    const int32_t ratio = gp4::idiv64(static_cast<int64_t>(remaining) * limited, GP4_GLOBAL(int32_t, 0x00603854), &remainder);

    const int32_t gain = static_cast<int32_t>(static_cast<uint32_t>(
        (static_cast<int64_t>(ratio) * GP4_GLOBAL(int32_t, 0x00603774)) >> 14));

    int32_t load = self->Unknown2F4 + self->Unknown2F8;
    if (load < 0) load = 0;
    if (load > 0x10000) load = 0x10000;
    const int32_t quarter = static_cast<int32_t>(static_cast<uint32_t>(load) >> 2);
    const int32_t blended = static_cast<int32_t>(static_cast<uint32_t>(
        (static_cast<int64_t>(gain) * quarter) >> 14));
    GP4_GLOBAL(int32_t, 0x00603778) = blended;

    const int32_t difference2 = static_cast<int32_t>(self->Unknown23C) - GP4_GLOBAL(int32_t, 0x00638d20);
    const int32_t limited2 = clampMagnitude(difference2, GP4_GLOBAL(uint32_t, 0x00603708));
    const int32_t correction = gp4::idiv64(static_cast<int64_t>(limited2) * GP4_GLOBAL(int32_t, 0x0060370c), GP4_GLOBAL(int32_t, 0x00603854), &remainder);
    GP4_GLOBAL(int32_t, 0x00603704) = correction;

    const int32_t total = GP4_GLOBAL(int32_t, 0x00603744) + GP4_GLOBAL(int32_t, 0x00603778) + GP4_GLOBAL(int32_t, 0x00603704);
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(total) * static_cast<int32_t>(self->Unknown173), 100, &remainder);

    // Clamp |quotient| to 10000 (unsigned compare) keeping the sign. The flags left behind are
    // those of the last cmp (non-negative) or of the final neg (negative).
    int32_t clamped;
    uint32_t flags;
    if (quotient < 0) {
        uint32_t magnitude = 0u - static_cast<uint32_t>(quotient);
        if (magnitude >= 0x2710u) magnitude = 0x2710u;
        const uint32_t negated = 0u - magnitude;
        flags = subFlags(0u, magnitude, negated);
        clamped = static_cast<int32_t>(negated);
    } else {
        uint32_t value = static_cast<uint32_t>(quotient);
        flags = subFlags(value, 0x2710u, value - 0x2710u);
        if (value >= 0x2710u) value = 0x2710u;
        clamped = static_cast<int32_t>(value);
    }
    self->Unknown5E = static_cast<int16_t>(clamped);
    self->Unknown23C = static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00638d20));

    r->ebp = 0x64u;
    r->edx = static_cast<uint32_t>(remainder);
    r->eax = (static_cast<uint32_t>(clamped) & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00638d20);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041a2ec, FUN_0041a2ec, "regs(esi:ptr[0x308]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0041a2ec")
