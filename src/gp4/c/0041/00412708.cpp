#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Signed 64-by-32 idiv quotient (assumed to fit; the original faults otherwise). The freestanding
// runtime has no __alldiv, so divide |dividend| by |divisor| bit by bit.
static inline int32_t idiv64(int64_t dividend, int32_t divisor) {
    const bool negativeDividend = dividend < 0;
    const uint64_t magnitude = negativeDividend ? 0u - (uint64_t)dividend : (uint64_t)dividend;
    const uint32_t d = divisor < 0 ? 0u - (uint32_t)divisor : (uint32_t)divisor;
    uint32_t rem = (uint32_t)(magnitude >> 32);
    uint32_t low = (uint32_t)magnitude;
    uint32_t q = 0;
    for (int i = 0; i < 32; i++) {
        const uint32_t carry = rem >> 31;
        rem = (rem << 1) | (low >> 31);
        low <<= 1;
        q <<= 1;
        if (carry != 0u || rem >= d) {
            rem -= d;
            q |= 1u;
        }
    }
    return (int32_t)((negativeDividend != (divisor < 0)) ? 0u - q : q);
}

// FUNCTION: GP4 0x00412708
void __cdecl FUN_00412708(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00603854);

    // 16x16 -> 32 unsigned multiply of the low word by 1000
    const uint32_t wide = (value & 0xffffu) * 1000u;
    const uint16_t high = (uint16_t)(wide >> 16);
    GP4_GLOBAL(uint16_t, 0x00679b56) = (uint16_t)wide;
    GP4_GLOBAL(uint16_t, 0x00679b14) = high;
    GP4_GLOBAL(uint16_t, 0x00679b16) = 0;
    GP4_GLOBAL(uint16_t, 0x00679b18) = high;
    GP4_GLOBAL(uint16_t, 0x00679b1a) = 0;

    // value * 1000 * [62d238] / ([62d238] + [62d23c]) with a 64-bit intermediate
    const int32_t numerator = (int32_t)(value * 1000u);
    const int32_t part = GP4_GLOBAL(int32_t, 0x0062d238);
    const int32_t total = (int32_t)((uint32_t)part + GP4_GLOBAL(uint32_t, 0x0062d23c));
    const int64_t product = (int64_t)numerator * (int64_t)part;
    const uint32_t quotient = (uint32_t)idiv64(product, total);
    GP4_GLOBAL(uint16_t, 0x00679b54) = (uint16_t)quotient;
    const uint32_t shifted = quotient >> 16;
    GP4_GLOBAL(uint32_t, 0x00679b10) = shifted;

    r->ebp = (uint32_t)total;
    // flags of the final "shr eax, 16" (AF=0, OF=bit31(prev ^ result)=0 in the oracle model)
    r->eflags = (r->eflags & ~0x8d5u)
        | parityFlag(shifted)
        | (shifted == 0u ? 0x40u : 0u)
        | ((quotient >> 15) & 1u);
}
GP4_IMPL(0x00412708, FUN_00412708, "regs() -> (ebp, cf, pf, af, zf, sf, of); globals=layout=state_00412708")
