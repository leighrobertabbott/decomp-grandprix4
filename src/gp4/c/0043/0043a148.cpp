#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left - right` whose result is `result` (cmp).
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Mode byte 0x006097fc selects the tracking direction: zero keeps the smallest sample seen, nonzero
// the largest. The sample is 0x00608298; each of the two signed values 0x00608290 / 0x00608294
// owns a pair of slots (non-negative: bound + value, negative: bound + value) updated when the
// sample beats the stored bound.
static inline bool beats(bool largest, int32_t bound, int32_t sample) {
    return largest ? bound < sample : bound > sample;
}

// FUNCTION: GP4 0x0043a148
void __cdecl FUN_0043a148(gp4::Regs* r) {
    const bool largest = GP4_GLOBAL(uint8_t, 0x006097fc) != 0u;

    const int32_t first = GP4_GLOBAL(int32_t, 0x00608290);
    if (first >= 0) {
        if (beats(largest, GP4_GLOBAL(int32_t, 0x00609808), GP4_GLOBAL(int32_t, 0x00608298))) {
            GP4_GLOBAL(int32_t, 0x00609808) = GP4_GLOBAL(int32_t, 0x00608298);
            GP4_GLOBAL(int32_t, 0x00609800) = first;
        }
    } else {
        if (beats(largest, GP4_GLOBAL(int32_t, 0x0060980c), GP4_GLOBAL(int32_t, 0x00608298))) {
            GP4_GLOBAL(int32_t, 0x0060980c) = GP4_GLOBAL(int32_t, 0x00608298);
            GP4_GLOBAL(int32_t, 0x00609804) = first;
        }
    }

    const int32_t second = GP4_GLOBAL(int32_t, 0x00608294);
    int32_t bound;
    uint32_t flags;
    if (second >= 0) {
        bound = GP4_GLOBAL(int32_t, 0x00609818);
        const int32_t sample = GP4_GLOBAL(int32_t, 0x00608298);
        flags = subFlags(static_cast<uint32_t>(bound), static_cast<uint32_t>(sample), static_cast<uint32_t>(bound) - static_cast<uint32_t>(sample));
        if (beats(largest, bound, sample)) {
            bound = GP4_GLOBAL(int32_t, 0x00608298);
            GP4_GLOBAL(int32_t, 0x00609818) = bound;
            GP4_GLOBAL(int32_t, 0x00609810) = second;
        }
    } else {
        bound = GP4_GLOBAL(int32_t, 0x0060981c);
        const int32_t sample = GP4_GLOBAL(int32_t, 0x00608298);
        flags = subFlags(static_cast<uint32_t>(bound), static_cast<uint32_t>(sample), static_cast<uint32_t>(bound) - static_cast<uint32_t>(sample));
        if (beats(largest, bound, sample)) {
            bound = GP4_GLOBAL(int32_t, 0x00608298);
            GP4_GLOBAL(int32_t, 0x0060981c) = bound;
            GP4_GLOBAL(int32_t, 0x00609814) = second;
        }
    }

    // eax leaves holding the second value, edx the bound (or the sample that replaced it), and
    // the flags are those of the last "cmp edx, [0x608298]"
    r->eax = static_cast<uint32_t>(second);
    r->edx = static_cast<uint32_t>(bound);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043a148, FUN_0043a148, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
