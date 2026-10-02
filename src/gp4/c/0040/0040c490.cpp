#include <gp4/gp4.h>

static inline uint32_t AddFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return static_cast<uint32_t>((static_cast<uint64_t>(lhs) + rhs) >> 32)
        | ((~parity & 1u) << 2) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x0040c490
void __cdecl FUN_0040c490(gp4::Regs* r) {
    const int32_t x = GP4_GLOBAL(int32_t, 0x0063b820);
    const int32_t y = GP4_GLOBAL(int32_t, 0x0063b824);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x0062ac74);
    const int64_t xProduct = static_cast<int64_t>(x >> 1) * scale;
    const int64_t yProduct = static_cast<int64_t>(y >> 1) * scale;
    const uint32_t xTerm = static_cast<uint32_t>(static_cast<uint64_t>(xProduct) >> 14);
    const uint32_t yTerm = static_cast<uint32_t>(static_cast<uint64_t>(yProduct) >> 14);
    GP4_GLOBAL(uint32_t, 0x0063b828) -= xTerm;
    const uint32_t previous = GP4_GLOBAL(uint32_t, 0x0063b82c);
    const uint32_t result = previous + yTerm;
    GP4_GLOBAL(uint32_t, 0x0063b82c) = result;
    r->eflags = (r->eflags & ~0x8d5u) | AddFlags(previous, yTerm, result);
}
GP4_IMPL(0x0040c490, FUN_0040c490, "regs() -> (cf,pf,af,zf,sf,of); globals=layout=state_0040c490")
