#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Signed (hi:lo) / divisor as the x86 idiv does (truncating; the CPU faults when the quotient
// does not fit). Written out so no CRT 64-bit division helper is called.
static inline int32_t idiv64by32(int64_t dividend, int32_t divisor) {
    const bool negative = (dividend < 0) != (divisor < 0);
    const uint64_t magnitude = dividend < 0 ? 0u - static_cast<uint64_t>(dividend) : static_cast<uint64_t>(dividend);
    const uint32_t d = divisor < 0 ? 0u - static_cast<uint32_t>(divisor) : static_cast<uint32_t>(divisor);
    const uint32_t hi = static_cast<uint32_t>(magnitude >> 32);
    const uint32_t lo = static_cast<uint32_t>(magnitude);
    uint32_t rem = hi;
    uint32_t quot = 0u;
    for (int bit = 31; bit >= 0; bit--) {
        const uint32_t carry = rem >> 31;
        rem = (rem << 1) | ((lo >> bit) & 1u);
        quot <<= 1;
        if (carry != 0u || rem >= d) {
            rem -= d;
            quot |= 1u;
        }
    }
    return static_cast<int32_t>(negative ? 0u - quot : quot);
}

// FUNCTION: GP4 0x00419afd
void __cdecl FUN_00419afd(gp4::Regs* r) {
    // Sum of four per-slot values (ebx = 0x6380d8; +0x274/+0x574/+0x874/+0xb74), clamped to +-0x1000000.
    uint32_t sum = GP4_GLOBAL(uint32_t, 0x0063834c) + GP4_GLOBAL(uint32_t, 0x0063864c)
        + GP4_GLOBAL(uint32_t, 0x0063894c) + GP4_GLOBAL(uint32_t, 0x00638c4c);
    if ((sum & 0x80000000u) != 0u) {
        sum = 0u - sum;
        if (sum >= 0x1000000u) {
            sum = 0x1000000u;
        }
        sum = 0u - sum;
    } else if (sum >= 0x1000000u) {
        sum = 0x1000000u;
    }

    // cdq; shld edx, eax, 11; shl eax, 11; idiv [0x601380]
    const int64_t dividend = static_cast<int64_t>(static_cast<int32_t>(sum)) << 11;
    const int32_t quotient = idiv64by32(dividend, GP4_GLOBAL(int32_t, 0x00601380));

    // imul [0x6f2bf0]; shrd eax, edx, 14
    const int64_t product = static_cast<int64_t>(quotient) * GP4_GLOBAL(int32_t, 0x006f2bf0);
    const uint32_t step = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x006228e0);
    GP4_GLOBAL(uint32_t, 0x006259e4) = before;
    GP4_GLOBAL(uint32_t, 0x006259e8) = step;
    const uint32_t after = before + step;
    GP4_GLOBAL(uint32_t, 0x006228e0) = after;

    // the final "add [0x6228e0], eax" decides every flag
    const uint32_t flags = (after < before ? 1u : 0u)
        | parityFlag(after)
        | (((before ^ step ^ after) & 0x10u) != 0u ? 0x10u : 0u)
        | (after == 0u ? 0x40u : 0u)
        | ((after & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(before ^ step) & (before ^ after) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->ebx = 0x006380d8u;
    r->eax = step;
    r->edx = before;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00419afd, FUN_00419afd, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=state_00419afd")
