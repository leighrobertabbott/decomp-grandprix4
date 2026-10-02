#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00429395
void __cdecl FUN_00429395(gp4::Regs* r) {
    const int32_t first = GP4_GLOBAL(int32_t, 0x00601b38);
    const int32_t last = GP4_GLOBAL(int32_t, 0x00601b40);
    int32_t value = GP4_GLOBAL(int32_t, 0x00601b3c);
    int32_t compared;
    int32_t against;
    if (first < last) {
        if (value < first) value = first;
        compared = value;
        against = last;
        if (value > last) value = last;
    } else {
        if (value < last) value = last;
        compared = value;
        against = first;
        if (value > first) value = first;
    }
    GP4_GLOBAL(int32_t, 0x00601b3c) = value;
    r->eax = static_cast<uint32_t>(value);
    r->edx = static_cast<uint32_t>(first);
    r->eflags = (r->eflags & ~0x8d5u)
        | subtractionFlags32(static_cast<uint32_t>(compared), static_cast<uint32_t>(against));
}
GP4_IMPL(0x00429395, FUN_00429395, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
