#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit "cmp left, right".
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Blends the word at esi+0x1b6 towards q, the ratio (<< 14) of the sum of the dwords at
// +0x2f4/+0x2f8 to the sum of the four dwords at +0x2ec..+0x2f8 (1 when that sum is zero):
// new = (old * (0x4000 - w) + q * w) / 2^14 rounded, with w = ([0x60384c] * [0x006030dc]) >> 14,
// clamped to [[0x006030e0], [0x006030e4]] (signed). eax = result, ecx = 0x4000 - w, edx/ebp = high
// halves of the 64-bit sums; the flags are those of the last `cmp eax, [0x006030e4]`.
// FUNCTION: GP4 0x0041325b
void __cdecl FUN_0041325b(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(r->esi);

    const uint32_t numerator = GP4_FIELD(uint32_t, body, 0x2f4) + GP4_FIELD(uint32_t, body, 0x2f8);
    uint32_t divisor = GP4_FIELD(uint32_t, body, 0x2ec) + GP4_FIELD(uint32_t, body, 0x2f0) + numerator;
    if (divisor == 0u) {
        divisor = 1u;
    }
    const int64_t dividend = static_cast<int64_t>(static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(numerator))) << 14);
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(dividend, static_cast<int32_t>(divisor), &remainder);

    const int64_t rateProduct = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060384c)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006030dc));
    const uint32_t weight = static_cast<uint32_t>(static_cast<uint64_t>(rateProduct) >> 14);
    const uint64_t towards = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(weight)) * static_cast<int64_t>(quotient));
    const uint32_t complement = 0x4000u - weight;
    const uint32_t old = GP4_FIELD(uint16_t, body, 0x1b6);
    const uint64_t kept = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(old)) * static_cast<int64_t>(static_cast<int32_t>(complement)));
    const uint64_t total = kept + towards;

    const uint32_t shifted = static_cast<uint32_t>(total >> 13);
    int32_t value = static_cast<int32_t>(static_cast<int32_t>(shifted) >> 1) + static_cast<int32_t>(shifted & 1u);

    const int32_t lower = GP4_GLOBAL(int32_t, 0x006030e0);
    const int32_t upper = GP4_GLOBAL(int32_t, 0x006030e4);
    if (!(value >= lower)) {
        value = lower;
    }
    const uint32_t flags = subFlags(static_cast<uint32_t>(value), static_cast<uint32_t>(upper));
    if (!(value <= upper)) {
        value = upper;
    }
    GP4_FIELD(uint16_t, body, 0x1b6) = static_cast<uint16_t>(value);

    r->eax = static_cast<uint32_t>(value);
    r->ecx = complement;
    r->edx = static_cast<uint32_t>(total >> 32);
    r->ebp = static_cast<uint32_t>(towards >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041325b, FUN_0041325b, "regs(esi:ptr[0x300]:layout=rec_0041325b) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
