#include <gp4/gp4.h>

// (value << 14) / divisor for 0 <= value <= divisor, divisor >= 1: the original's cdq/shld/shl/idiv
// 64-by-32 division. The freestanding runtime has no __alldiv, so do the 14 extra bits by long division.
static inline int32_t divideShifted14(uint32_t value, uint32_t divisor) {
    uint32_t quotient = value / divisor;
    uint32_t remainder = value % divisor;
    quotient <<= 14;
    for (int bit = 13; bit >= 0; bit--) {
        remainder <<= 1;
        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= 1u << bit;
        }
    }
    return static_cast<int32_t>(quotient);
}

// FUNCTION: GP4 0x004384e7
void __cdecl FUN_004384e7(gp4::Regs* r) {
    const int32_t stored = GP4_GLOBAL(int32_t, 0x00629ec4);
    int32_t divisor = stored;
    if (divisor < 1) {
        divisor = 1;
    }
    int32_t remaining = static_cast<int32_t>(static_cast<uint32_t>(stored)
        - (GP4_GLOBAL(uint32_t, 0x0062d2a8) << 8));
    if (remaining < 0) {
        remaining = 0;
    }
    GP4_GLOBAL(int32_t, 0x00629ec4) = remaining;
    if (remaining > divisor) {
        remaining = divisor;
    }
    const int32_t ratio = divideShifted14(static_cast<uint32_t>(remaining), static_cast<uint32_t>(divisor));

    const int64_t first = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00629ed0)) * ratio;
    GP4_GLOBAL(uint32_t, 0x00629ed0) = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);

    const int64_t second = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00629ed4)) * ratio;
    const uint64_t bits = static_cast<uint64_t>(second);
    const uint32_t result = static_cast<uint32_t>(bits >> 14);
    GP4_GLOBAL(uint32_t, 0x00629ed4) = result;

    // Flags come from the final "shrd eax, edx, 14". OF/AF are architecturally undefined
    // there: pinned oracle model (CF = bit 13 of eax, AF = 0, OF = bit31(prev ^ result)).
    const uint32_t previous = static_cast<uint32_t>(bits >> 13);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = static_cast<uint32_t>((bits >> 13) & 1u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((previous ^ result) >> 31) << 11);
    r->eax = result;
    r->edx = static_cast<uint32_t>(bits >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004384e7, FUN_004384e7, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
