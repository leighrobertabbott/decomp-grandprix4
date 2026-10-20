#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t resultFlags(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Integer square root of the unsigned 64-bit value edx:eax (result in eax). With edx == 0 it is
// the 32-bit root at 0x004010f3. Otherwise the value is normalised by shifts of 2 until its top
// two bits are not both clear, halved, and refined by four Newton steps (each a 64/32 division
// by the running estimate), then shifted back. Every register except eax is preserved.
// FUNCTION: GP4 0x0040113f
void __cdecl FUN_0040113f(gp4::Regs* r) {
    uint32_t high = r->edx;
    uint32_t low = r->eax;

    if (high == 0u) {
        gp4::Regs call = *r;
        gp4::call_regs(0x004010f3, call);
        r->eax = call.eax;
        r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
        return;
    }

    uint32_t shifts = 0;
    // cmp edx, 0x40000000 / inc ebp / shld edx, eax, 2 / shl eax, 2 until the top two bits are set
    while (high < 0x40000000u) {
        ++shifts;
        high = (high << 2) | (low >> 30);
        low <<= 2;
    }

    // shrd eax, edx, 1 ; shr edx, 1
    low = (low >> 1) | (high << 31);
    high >>= 1;

    const uint32_t dividendLow = low;
    const uint32_t dividendHigh = high;
    uint32_t estimate = high + 0x80000000u;
    uint32_t flags = 0;
    for (int i = 0; i < 4; ++i) {
        uint32_t remainder;
        const uint32_t quotient = gp4::div64((static_cast<uint64_t>(dividendHigh) << 32) | dividendLow, estimate, &remainder);
        const uint32_t left = estimate >> 1;
        const uint32_t sum = left + quotient;
        // add edi, eax
        flags = (sum < left ? 1u : 0u)
            | resultFlags(sum)
            | (((left ^ quotient ^ sum) & 0x10u) != 0u ? 0x10u : 0u)
            | ((~(left ^ quotient) & (left ^ sum) & 0x80000000u) != 0u ? 0x800u : 0u);
        estimate = sum;
    }

    uint32_t result = estimate;
    if (shifts != 0u) {
        // shr edi, cl
        result = estimate >> shifts;
        flags = ((estimate >> (shifts - 1u)) & 1u)
            | resultFlags(result)
            | ((((estimate >> (shifts - 1u)) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040113f, FUN_0040113f, "regs(eax:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of)")
