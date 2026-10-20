#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040c51a
void __cdecl FUN_0040c51a(gp4::Regs* r) {
    // Clamp eax to [0, 0x400000] (signed) and store it at 0x62ac74.
    // The flags left behind are those of "cmp eax, 0x400000" on the already floored value.
    int32_t value = static_cast<int32_t>(r->eax);
    if (value < 0) {
        value = 0;
    }
    const uint32_t flags = subFlags(static_cast<uint32_t>(value), 0x400000u);
    if (value > 0x400000) {
        value = 0x400000;
    }
    GP4_GLOBAL(int32_t, 0x0062ac74) = value;
    r->eax = static_cast<uint32_t>(value);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040c51a, FUN_0040c51a, "regs(eax:i32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
