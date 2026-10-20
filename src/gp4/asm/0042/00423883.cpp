#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right` (cmp).
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Flags of the 32-bit `neg value`.
static inline uint32_t negFlags(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u) | parityFlag(result) | ((value ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

// Flags of `or value, value` / `or edx, edx` (CF = OF = AF = 0).
static inline uint32_t orFlags(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value >> 24) & 0x80u);
}

// Clamps the signed 64-bit value edx:eax to the range [-ebp, ebp] and returns the low dword in
// eax. The flags are those of the last flag-setting instruction executed (cmp, or, neg).
// FUNCTION: GP4 0x00423883
void __cdecl FUN_00423883(gp4::Regs* r) {
    const uint32_t high = r->edx;
    const uint32_t low = r->eax;
    const uint32_t limit = r->ebp;

    uint32_t result;
    uint32_t flags;

    if (static_cast<int32_t>(high) >= 0) {
        flags = cmpFlags(high, 0u);
        if (high != 0u) {
            result = limit;
        } else {
            flags = cmpFlags(low, limit);
            result = low <= limit ? low : limit;
        }
    } else {
        flags = cmpFlags(high, 0xffffffffu);
        bool clampLow = true;
        uint32_t magnitude = 0u;
        if (high == 0xffffffffu) {
            flags = orFlags(low);
            if (low != 0u) {
                magnitude = 0u - low;
                flags = negFlags(low);
                flags = cmpFlags(magnitude, limit);
                clampLow = magnitude > limit;
            }
        }
        if (clampLow) {
            result = 0u - limit;
            flags = negFlags(limit);
        } else {
            result = 0u - magnitude;
            flags = negFlags(magnitude);
        }
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00423883, FUN_00423883, "regs(eax:u32, edx:i32[-2..2], ebp:u32) -> (eax, cf, pf, af, zf, sf, of)")
