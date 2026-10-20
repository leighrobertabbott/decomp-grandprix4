#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right" (also "neg x" as cmp 0, x)
static inline uint32_t compare32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// flags of "test value, value" (CF = OF = AF = 0)
static inline uint32_t test32Flags(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0043c0cd
// Fraction (signed 8.8 fixed point, clamped to -0x100..0x100) of where ebp lies between eax and
// edx: (edx - eax) * 256 / (2 * max(|ebp - eax|, |ebp - edx|)). Out-of-order samples saturate to
// +-0x100. ebx, ecx and edi are saved and restored; edx is left as the remainder or ebp - (the
// compared sample); flags are those of the last compare/neg.
void __cdecl FUN_0043c0cd(gp4::Regs* r) {
    const uint32_t a = r->eax;     // ebx
    const uint32_t d = r->edx;     // edi
    const uint32_t p = r->ebp;

    uint32_t eax;
    uint32_t edx;
    uint32_t flags;

    const uint32_t ap = a - p;
    const uint32_t pd = p - d;
    if (static_cast<int32_t>(ap) >= 0 && ap != 0u && !(static_cast<int32_t>(ap) < static_cast<int32_t>(pd))) {
        edx = pd;
        flags = compare32Flags(a, d);
        eax = static_cast<int32_t>(a) < static_cast<int32_t>(d) ? 0x100u : 0xffffff00u;
    } else {
        const uint32_t dp = d - p;
        const uint32_t pa = p - a;
        if (static_cast<int32_t>(dp) >= 0 && dp != 0u && !(static_cast<int32_t>(dp) < static_cast<int32_t>(pa))) {
            edx = pa;
            flags = compare32Flags(d, a);
            eax = static_cast<int32_t>(d) < static_cast<int32_t>(a) ? 0xffffff00u : 0x100u;
        } else {
            uint32_t distA = pa;       // |ebp - ebx|
            if (static_cast<int32_t>(distA) < 0) distA = 0u - distA;
            uint32_t distD = pd;       // |ebp - edi|
            if (static_cast<int32_t>(distD) < 0) distD = 0u - distD;
            uint32_t span = static_cast<int32_t>(distA) < static_cast<int32_t>(distD) ? distD : distA;
            span <<= 1;
            if (span == 0u) span = 1u;

            const int64_t dividend = static_cast<int64_t>(static_cast<int32_t>(d - a)) << 8;
            int32_t remainder;
            const uint32_t quotient = static_cast<uint32_t>(gp4::idiv64(dividend, static_cast<int32_t>(span), &remainder));
            edx = static_cast<uint32_t>(remainder);

            if (static_cast<int32_t>(quotient) < 0) {
                uint32_t mag = 0u - quotient;
                if (mag >= 0x100u) mag = 0x100u;
                flags = compare32Flags(0u, mag);
                eax = 0u - mag;
            } else {
                flags = compare32Flags(quotient, 0x100u);
                eax = quotient >= 0x100u ? 0x100u : quotient;
            }
        }
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043c0cd, FUN_0043c0cd, "regs(eax:i32[-8..8], ebp:i32[-8..8], edx:i32[-8..8]) -> (eax, edx, cf, pf, af, zf, sf, of)")
