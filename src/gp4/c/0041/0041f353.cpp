#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of "sub/cmp left, right" (CF, PF, AF, ZF, SF, OF).
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The magnitude is limited to 0x80000 (unsigned compare after neg for negative values).
static inline int32_t clampMagnitude(int32_t value) {
    if (value < 0) {
        uint32_t magnitude = 0u - static_cast<uint32_t>(value);
        if (magnitude >= 0x80000u) {
            magnitude = 0x80000u;
        }
        return static_cast<int32_t>(0u - magnitude);
    }
    uint32_t magnitude = static_cast<uint32_t>(value);
    if (magnitude >= 0x80000u) {
        magnitude = 0x80000u;
    }
    return static_cast<int32_t>(magnitude);
}

// FUNCTION: GP4 0x0041f353
// Uses the four dwords +0x174, +0x474, +0x774, +0xa74 of the 0x006380d8 block (two pairs).
// Subtracts from 0x00638044, 0x00638034 and 0x00638038 quotients of the pair sums/differences
// scaled by 0x00601350 / 0x0060134c / 0x00601334 and the divisors 0x00638da4+0x00638da8,
// 0x00638dac and 0x00638db0. Leaves ebx = 0x006380d8, ecx = second pair average,
// ebp = clamped second difference, edx = third quotient, eax = last sum and the flags of
// the final "sub [0x00638038], eax".
void __cdecl FUN_0041f353(gp4::Regs* r) {
    uint8_t* block = GP4_ARRAY(uint8_t, 0x006380d8);
    int32_t rem;

    const int32_t averageA = static_cast<int32_t>(GP4_FIELD(uint32_t, block, 0x174) + GP4_FIELD(uint32_t, block, 0x474)) >> 1;
    const int32_t quotient1 = gp4::idiv64(static_cast<int64_t>(averageA) * GP4_GLOBAL(int32_t, 0x00601350),
                                          GP4_GLOBAL(int32_t, 0x00601334), &rem);
    const int32_t averageB = static_cast<int32_t>(GP4_FIELD(uint32_t, block, 0x774) + GP4_FIELD(uint32_t, block, 0xa74)) >> 1;
    const int32_t quotient2 = gp4::idiv64(static_cast<int64_t>(averageB) * GP4_GLOBAL(int32_t, 0x0060134c),
                                          GP4_GLOBAL(int32_t, 0x00601334), &rem);
    GP4_GLOBAL(int32_t, 0x00638044) -= static_cast<int32_t>(static_cast<uint32_t>(quotient2) + static_cast<uint32_t>(quotient1)) >> 4;

    const int32_t spread = clampMagnitude(static_cast<int32_t>(static_cast<uint32_t>(averageB) - static_cast<uint32_t>(averageA)));
    const int32_t divisor = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00638da8) + GP4_GLOBAL(uint32_t, 0x00638da4));
    GP4_GLOBAL(int32_t, 0x00638034) -= gp4::idiv64(static_cast<int64_t>(spread) << 26, divisor, &rem);

    const int32_t diffX = clampMagnitude(static_cast<int32_t>(GP4_FIELD(uint32_t, block, 0x174) - GP4_FIELD(uint32_t, block, 0x474)));
    const int32_t scaledX = gp4::idiv64(static_cast<int64_t>(diffX) * GP4_GLOBAL(int32_t, 0x00601350),
                                        GP4_GLOBAL(int32_t, 0x00601334), &rem);
    const int32_t quotient4 = gp4::idiv64(static_cast<int64_t>(scaledX) << 25, GP4_GLOBAL(int32_t, 0x00638dac), &rem);

    const int32_t diffZ = clampMagnitude(static_cast<int32_t>(GP4_FIELD(uint32_t, block, 0x774) - GP4_FIELD(uint32_t, block, 0xa74)));
    const int32_t scaledZ = gp4::idiv64(static_cast<int64_t>(diffZ) * GP4_GLOBAL(int32_t, 0x0060134c),
                                        GP4_GLOBAL(int32_t, 0x00601334), &rem);
    const int32_t quotient5 = gp4::idiv64(static_cast<int64_t>(scaledZ) << 25, GP4_GLOBAL(int32_t, 0x00638db0), &rem);

    const uint32_t total = static_cast<uint32_t>(quotient5) + static_cast<uint32_t>(quotient4);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00638038);
    GP4_GLOBAL(uint32_t, 0x00638038) = before - total;

    r->eax = total;
    r->ebx = 0x006380d8u;
    r->ecx = static_cast<uint32_t>(averageB);
    r->edx = static_cast<uint32_t>(quotient4);
    r->ebp = static_cast<uint32_t>(diffZ);
    r->eflags = (r->eflags & ~0x8d5u) | subtractFlags32(before, total);
}
GP4_IMPL(0x0041f353, FUN_0041f353, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=recs_006380d8")
