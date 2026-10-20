#include <gp4/gp4.h>

struct UnknownRecord_0041a463 {
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

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `shrd eax, edx, 14` on the 64-bit product edx:eax in the oracle model:
// CF = last bit shifted out, AF = 0, OF = bit 31 of (product >> 13) ^ result.
static inline uint32_t shrd14Flags(uint64_t product) {
    const uint32_t result = static_cast<uint32_t>(product >> 14);
    const uint32_t previous = static_cast<uint32_t>(product >> 13);
    return static_cast<uint32_t>((product >> 13) & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) & 0x80000000u) != 0u) ? 0x800u : 0u);
}

// (a * b) >> 14, the low dword ("imul; shrd eax, edx, 14").
static inline int32_t mulShift14(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 14));
}

static inline int32_t absolute(int32_t value) {
    return value < 0 ? static_cast<int32_t>(0u - static_cast<uint32_t>(value)) : value;
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

// FUNCTION: GP4 0x0041a463
// Runs 0x0041a619, then the same rate-scaled blend as 0x0041a2ec: level = min(max(|[0x638ce0]|,
// |[0x638cd8]|) >> 8, 0x4000), q1 = (0x4000 - min(level * [0x60377c] >> 14, 0x4000)) *
// clamp(word esi+0x23c - [0x638d20], +-0x4000) / [0x603854], [0x603778] = (q1 * [0x603774] >> 14) *
// (clamp(esi+0x2f4 + esi+0x2f8, 0, 0x10000) >> 2) >> 14, [0x603784] = clamp(level * [0x603768] >> 14
// + [0x60376c], 0, 0x4000), [0x603770] = clamp(|[0x638d20]| * [0x603760] >> 14 + [0x603764] -
// [0x603784], 0, 0x4000) and word esi+0x5e = clamp((([0x603770] * [0x603744] >> 14) + [0x603778]) *
// [0x603780] >> 14 * byte esi+0x173 / 100, +-10000) * [0x60373c] >> 14, with [0x603780] = 0x4a00 -
// [0x603854]; word esi+0x23c takes [0x638d20]. eax = high half of the last product | word
// [0x638d20], edx = its high dword, ebp = 100, flags of the last shrd.
void __cdecl FUN_0041a463(gp4::Regs* r) {
    UnknownRecord_0041a463* self = reinterpret_cast<UnknownRecord_0041a463*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0041a619, call);

    GP4_GLOBAL(int32_t, 0x00603780) = 0x4a00 - GP4_GLOBAL(int32_t, 0x00603854);

    int32_t first = absolute(GP4_GLOBAL(int32_t, 0x00638ce0));
    const int32_t second = absolute(GP4_GLOBAL(int32_t, 0x00638cd8));
    if (first < second) first = second;
    int32_t level = static_cast<int32_t>(static_cast<uint32_t>(first) >> 8);
    if (level > 0x4000) level = 0x4000;

    int32_t grip = mulShift14(level, GP4_GLOBAL(int32_t, 0x0060377c));
    if (grip > 0x4000) grip = 0x4000;
    const int32_t remaining = static_cast<int32_t>(0x4000u - static_cast<uint32_t>(grip));

    const int32_t difference = static_cast<int32_t>(static_cast<uint32_t>(static_cast<int32_t>(self->Unknown23C))
        - GP4_GLOBAL(uint32_t, 0x00638d20));
    const int32_t limited = clampMagnitude(difference, 0x4000u);
    int32_t remainder = 0;
    const int32_t ratio = gp4::idiv64(static_cast<int64_t>(remaining) * limited, GP4_GLOBAL(int32_t, 0x00603854), &remainder);
    const int32_t gain = mulShift14(ratio, GP4_GLOBAL(int32_t, 0x00603774));

    int32_t load = static_cast<int32_t>(static_cast<uint32_t>(self->Unknown2F4) + static_cast<uint32_t>(self->Unknown2F8));
    if (load < 0) load = 0;
    if (load > 0x10000) load = 0x10000;
    const int32_t quarter = static_cast<int32_t>(static_cast<uint32_t>(load) >> 2);
    GP4_GLOBAL(int32_t, 0x00603778) = mulShift14(gain, quarter);

    int32_t bias = static_cast<int32_t>(static_cast<uint32_t>(mulShift14(level, GP4_GLOBAL(int32_t, 0x00603768)))
        + GP4_GLOBAL(uint32_t, 0x0060376c));
    if (bias < 0) bias = 0;
    if (bias > 0x4000) bias = 0x4000;
    GP4_GLOBAL(int32_t, 0x00603784) = bias;

    int32_t blend = static_cast<int32_t>(static_cast<uint32_t>(mulShift14(absolute(GP4_GLOBAL(int32_t, 0x00638d20)), GP4_GLOBAL(int32_t, 0x00603760)))
        + GP4_GLOBAL(uint32_t, 0x00603764) - GP4_GLOBAL(uint32_t, 0x00603784));
    if (blend > 0x4000) blend = 0x4000;
    if (blend < 0) blend = 0;
    GP4_GLOBAL(int32_t, 0x00603770) = blend;

    const int32_t mixed = static_cast<int32_t>(static_cast<uint32_t>(mulShift14(blend, GP4_GLOBAL(int32_t, 0x00603744)))
        + GP4_GLOBAL(uint32_t, 0x00603778));
    const int32_t scaled = mulShift14(mixed, GP4_GLOBAL(int32_t, 0x00603780));
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(scaled) * static_cast<int32_t>(self->Unknown173), 100, &remainder);
    const int32_t clamped = clampMagnitude(quotient, 0x2710u);

    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(clamped) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060373c)));
    const uint32_t result = static_cast<uint32_t>(product >> 14);
    self->Unknown5E = static_cast<int16_t>(result);
    const uint16_t angle = GP4_GLOBAL(uint16_t, 0x00638d20);
    self->Unknown23C = static_cast<int16_t>(angle);

    r->eax = (result & 0xffff0000u) | angle;
    r->edx = static_cast<uint32_t>(product >> 32);
    r->ebp = 0x64u;
    r->eflags = (r->eflags & ~0x8d5u) | shrd14Flags(product);
}
GP4_IMPL(0x0041a463, FUN_0041a463, "regs(esi:ptr[0x308]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041a2ec+state_0041a619")
