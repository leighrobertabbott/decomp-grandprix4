#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Signed 64-bit edx:eax divided by the signed 32-bit ebx, giving a 64-bit quotient in edx:eax
// (truncated toward zero, but approximate when the high dword reaches the divisor: the dividend
// is then shifted right until it fits a single DIV and the 32-bit quotient is shifted back left).
// ebx is returned as |ebx|; ecx is preserved; the flags are those of the final sign fix-up.
// ebx == 0 never terminates in the original (the shift loop compares against zero), so it is not
// a valid input.
// FUNCTION: GP4 0x00402c7a
void __cdecl FUN_00402c7a(gp4::Regs* r) {
    uint32_t low = r->eax;
    uint32_t high = r->edx;
    uint32_t divisor = r->ebx;
    const uint32_t sign = high ^ divisor;

    if (static_cast<int32_t>(divisor) < 0) {
        divisor = 0u - divisor;
    }
    if (static_cast<int32_t>(high) < 0) {
        // 64-bit negate of the dividend
        high = ~high;
        low = 0u - low;
        if (low == 0u) {
            ++high;
        }
    }

    uint32_t remainder;
    uint32_t quotientLow;
    uint32_t quotientHigh = 0;
    if (high < divisor) {
        quotientLow = gp4::div64((static_cast<uint64_t>(high) << 32) | low, divisor, &remainder);
    } else {
        uint32_t shifts = 0;
        do {
            low = (low >> 1) | (high << 31);
            high >>= 1;
            ++shifts;
        } while (high >= divisor);
        quotientLow = gp4::div64((static_cast<uint64_t>(high) << 32) | low, divisor, &remainder);
        for (uint32_t n = shifts; n != 0u; --n) {
            quotientHigh = (quotientHigh << 1) | (quotientLow >> 31);
            quotientLow <<= 1;
        }
    }

    uint32_t flags;
    if (static_cast<int32_t>(sign) >= 0) {
        // or ecx, ecx
        flags = parityFlag(sign) | (sign == 0u ? 0x40u : 0u);
    } else {
        // not edx ; neg eax ; (inc edx when eax was zero)
        quotientHigh = ~quotientHigh;
        const uint32_t original = quotientLow;
        quotientLow = 0u - original;
        if (original != 0u) {
            flags = (1u) | parityFlag(quotientLow) | ((original ^ quotientLow) & 0x10u)
                | ((quotientLow & 0x80000000u) != 0u ? 0x80u : 0u)
                | (original == 0x80000000u ? 0x800u : 0u);
        } else {
            // neg 0 leaves CF = 0; the inc then rewrites PF/AF/ZF/SF/OF and keeps CF
            const uint32_t before = quotientHigh;
            ++quotientHigh;
            flags = parityFlag(quotientHigh) | ((before ^ 1u ^ quotientHigh) & 0x10u)
                | (quotientHigh == 0u ? 0x40u : 0u)
                | ((quotientHigh & 0x80000000u) != 0u ? 0x80u : 0u)
                | (before == 0x7fffffffu ? 0x800u : 0u);
        }
    }

    r->eax = quotientLow;
    r->edx = quotientHigh;
    r->ebx = divisor;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00402c7a, FUN_00402c7a, "regs(eax:u32, ebx:u32[1..0xffffffff], edx:u32) -> (eax, ebx, edx, cf, pf, af, zf, sf, of)")
