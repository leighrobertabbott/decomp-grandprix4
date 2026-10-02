#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// "idiv r32" on (high:low) / divisor, truncating toward zero. The original faults
// when the quotient does not fit; callers never produce that, so it is not modelled.
static inline int32_t signedDivide64By32(uint32_t high, uint32_t low, int32_t divisor) {
    const bool negativeDividend = (high & 0x80000000u) != 0u;
    const bool negativeDivisor = divisor < 0;
    if (negativeDividend) {
        const uint32_t borrow = (low == 0u) ? 1u : 0u;
        low = 0u - low;
        high = ~high + borrow;
    }
    const uint32_t magnitudeDivisor = negativeDivisor ? 0u - static_cast<uint32_t>(divisor)
                                                      : static_cast<uint32_t>(divisor);
    uint32_t remainder = high;
    uint32_t quotient = 0;
    for (int bit = 0; bit < 32; bit++) {
        const uint32_t carry = remainder >> 31;
        remainder = (remainder << 1) | (low >> 31);
        low <<= 1;
        quotient <<= 1;
        if (carry != 0u || remainder >= magnitudeDivisor) {
            remainder -= magnitudeDivisor;
            quotient |= 1u;
        }
    }
    return static_cast<int32_t>((negativeDividend != negativeDivisor) ? 0u - quotient : quotient);
}

// FUNCTION: GP4 0x0042adb2
void __cdecl FUN_0042adb2(gp4::Regs* r) {
    uint32_t first = r->eax;
    uint32_t second = r->edx;

    if (static_cast<int32_t>(first) < 0) {
        first = 0u - first;
    }
    if (static_cast<int32_t>(second) < 0) {
        second = 0u - second;
    }

    uint32_t divisor = first + second;
    if (divisor == 0u) {
        divisor = 1u;
    }

    uint32_t difference = first - second;
    if (static_cast<int32_t>(difference) < 0) {
        difference = 0u - difference;
    }

    // cdq; shld edx, eax, 14; shl eax, 14 : (difference << 14) as a 64-bit signed dividend.
    const uint32_t high = (((static_cast<int32_t>(difference) < 0) ? 0xffffffffu : 0u) << 14)
        | (difference >> 18);
    const uint32_t previous = difference << 13;
    const uint32_t low = difference << 14;

    // Flags come from "shl eax, 14"; idiv and the pops leave them alone.
    const uint32_t flags = (previous >> 31)
        | parityFlag8(low)
        | (low == 0u ? 0x40u : 0u)
        | ((low >> 31) << 7)
        | ((((previous ^ low) >> 31) & 1u) << 11);

    r->eax = static_cast<uint32_t>(signedDivide64By32(high, low, static_cast<int32_t>(divisor)));
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042adb2, FUN_0042adb2, "regs(eax:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of)")
