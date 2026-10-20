#include <gp4/gp4.h>

// Truncating signed 64 / 32 division (the original's idiv); the freestanding
// build has no __alldiv, so this is a plain shift-and-subtract loop.
static inline int64_t idiv64by32(int64_t dividend, int32_t divisor) {
    const bool negative = (dividend < 0) != (divisor < 0);
    uint64_t numerator = dividend < 0 ? 0 - static_cast<uint64_t>(dividend) : static_cast<uint64_t>(dividend);
    const uint64_t denominator = divisor < 0 ? 0 - static_cast<uint64_t>(static_cast<int64_t>(divisor))
                                             : static_cast<uint64_t>(divisor);
    uint64_t remainder = 0;
    uint64_t quotient = 0;
    for (int i = 0; i < 64; i++) {
        remainder = (remainder << 1) | (numerator >> 63);
        numerator <<= 1;
        quotient <<= 1;
        if (remainder >= denominator) {
            remainder -= denominator;
            quotient |= 1;
        }
    }
    return negative ? static_cast<int64_t>(0 - quotient) : static_cast<int64_t>(quotient);
}

// FUNCTION: GP4 0x0041bce9
void __cdecl FUN_0041bce9(gp4::Regs* r) {
    const int32_t sum = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00638118) + GP4_GLOBAL(uint32_t, 0x00638418));
    const int32_t value = (sum >> 1) + GP4_GLOBAL(int32_t, 0x00638ce0);
    const int32_t divisor = GP4_GLOBAL(int32_t, 0x006013d8);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00623b6c);

    // edx:eax = value << 14, idiv by the divisor
    const int64_t dividend = static_cast<int64_t>(value) * 0x4000;
    const int32_t quotient = static_cast<int32_t>(idiv64by32(dividend, divisor));

    // imul by 0x3c, shrd by 6
    const int64_t scaled = static_cast<int64_t>(quotient) * 0x3c;
    const int32_t shifted = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 6));

    // imul by the scale, shrd by 14
    const int64_t product = static_cast<int64_t>(shifted) * static_cast<int64_t>(scale);
    const uint64_t bits = static_cast<uint64_t>(product);
    const uint32_t result = static_cast<uint32_t>(bits >> 14);
    GP4_GLOBAL(uint32_t, 0x006258a8) = result;

    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = static_cast<uint32_t>((bits >> 13) & 1u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((bits >> 44) ^ (bits >> 45)) & 1u) != 0u ? 0x800u : 0u);

    r->ebx = 0x006380d8;
    r->eax = result;
    r->edx = static_cast<uint32_t>(bits >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041bce9, FUN_0041bce9, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=state_0041bce9")
