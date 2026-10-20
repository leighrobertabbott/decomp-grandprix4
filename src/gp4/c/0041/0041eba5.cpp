#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of add left, right
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// Signed 64-by-32 idiv (quotient assumed to fit; the original faults otherwise). The freestanding
// runtime has no __alldiv, so divide |dividend| by |divisor| bit by bit.
static inline int32_t idiv64(int64_t dividend, int32_t divisor, int32_t* remainder) {
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
    *remainder = (int32_t)(negativeDividend ? 0u - rem : rem);
    return (int32_t)((negativeDividend != (divisor < 0)) ? 0u - q : q);
}

// FUNCTION: GP4 0x0041eba5
void __cdecl FUN_0041eba5(gp4::Regs* r) {
    const int32_t ref = GP4_GLOBAL(int32_t, 0x0062785c);
    int32_t remainder;

    // first channel: [637ff0] moves towards [637ff4] by (span1 - ref) / max(span1, 16)
    const int32_t span1 = GP4_GLOBAL(int32_t, 0x00627858);
    const int32_t from1 = GP4_GLOBAL(int32_t, 0x00637ff0);
    const int64_t product1 = (int64_t)(int32_t)((uint32_t)span1 - (uint32_t)ref)
        * (int64_t)(int32_t)((uint32_t)GP4_GLOBAL(int32_t, 0x00637ff4) - (uint32_t)from1);
    const int32_t divisor1 = span1 < 0x10 ? 0x10 : span1;
    GP4_GLOBAL(int32_t, 0x00637ff0) = (int32_t)((uint32_t)from1 + (uint32_t)idiv64(product1, divisor1, &remainder));

    // second channel: [637ffc] moves towards [637ff8]
    const int32_t span2 = GP4_GLOBAL(int32_t, 0x00627860);
    const int32_t from2 = GP4_GLOBAL(int32_t, 0x00637ffc);
    const int64_t product2 = (int64_t)(int32_t)((uint32_t)span2 - (uint32_t)ref)
        * (int64_t)(int32_t)((uint32_t)GP4_GLOBAL(int32_t, 0x00637ff8) - (uint32_t)from2);
    const int32_t divisor2 = span2 < 0x10 ? 0x10 : span2;
    const int32_t quotient2 = idiv64(product2, divisor2, &remainder);
    GP4_GLOBAL(int32_t, 0x00637ffc) = (int32_t)((uint32_t)from2 + (uint32_t)quotient2);

    r->eax = (uint32_t)quotient2;
    r->edx = (uint32_t)remainder;
    r->ebp = (uint32_t)divisor2;
    // flags of the final "add [0x637ffc], eax"
    r->eflags = (r->eflags & ~0x8d5u) | addFlags((uint32_t)from2, (uint32_t)quotient2);
}
GP4_IMPL(0x0041eba5, FUN_0041eba5, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0041eba5")
