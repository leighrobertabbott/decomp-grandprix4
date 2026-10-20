#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "test reg/mem, 0xffffffff"
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// flags of "cmp left, right"
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004127b3
// Returns in eax a 0..0x4000 ratio min(|v|, w) * [0x6038e8] / w from the record at
// pointer global 0x00601a74 (v = +0xac, w = +0x284), or 0 when the sign of v does
// not agree with the sign of [0x6036dc] (flipped by bit 3 of [[rec+0x1c]+0x51]).
// ebp, edi, edx and esi are saved and restored.
void __cdecl FUN_004127b3(gp4::Regs* r) {
    uint8_t* const rec = GP4_GLOBAL(uint8_t*, 0x00601a74);
    const uint8_t* const block = GP4_FIELD(uint8_t*, rec, 0x1c);
    const bool flip = (block[0x51] & 0x08u) != 0u;
    const bool timeNegative = GP4_GLOBAL(int32_t, 0x006036dc) < 0;
    const int32_t value = GP4_FIELD(int32_t, rec, 0xac);
    const bool valueNegative = value < 0;

    uint32_t result;
    uint32_t flags;
    if (valueNegative != (flip != timeNegative)) {
        result = 0u;
        flags = logicFlags(static_cast<uint32_t>(value));
    } else {
        int32_t weight = GP4_FIELD(int32_t, rec, 0x284);
        int32_t magnitude = value;
        if (magnitude < 0) magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(magnitude));
        if (magnitude > weight) magnitude = weight;
        if (weight == 0) weight = 1;
        int32_t remainder = 0;
        int32_t quotient = gp4::idiv64(
            static_cast<int64_t>(magnitude) * GP4_GLOBAL(int32_t, 0x006038e8), weight, &remainder);
        if (quotient < 0) quotient = 0;
        flags = cmpFlags(static_cast<uint32_t>(quotient), 0x4000u);
        if (quotient > 0x4000) quotient = 0x4000;
        result = static_cast<uint32_t>(quotient);
    }
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004127b3, FUN_004127b3, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_004127b3")
