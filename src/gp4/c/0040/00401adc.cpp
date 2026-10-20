#include <gp4/gp4.h>

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// signed 32x32 -> 64 product, as "imul dword ptr [mem]" leaves it in edx:eax
static inline int64_t product(int32_t a, int32_t b) {
    return static_cast<int64_t>(a) * static_cast<int64_t>(b);
}

// "shrd eax, edx, 14": the low 32 bits of the product shifted right by 14
static inline uint32_t shifted(int64_t p) {
    return static_cast<uint32_t>(static_cast<uint64_t>(p) >> 14);
}

// FUNCTION: GP4 0x00401adc
void __cdecl FUN_00401adc(gp4::Regs* r) {
    // DAT_00623e24 = e18*e60 + e1c*e6c + e20*e78 (Q14)
    {
        const uint32_t partial = shifted(product(GP4_GLOBAL(int32_t, 0x00623e18), GP4_GLOBAL(int32_t, 0x00623e60)))
            + shifted(product(GP4_GLOBAL(int32_t, 0x00623e1c), GP4_GLOBAL(int32_t, 0x00623e6c)));
        GP4_GLOBAL(uint32_t, 0x00623e24) =
            shifted(product(GP4_GLOBAL(int32_t, 0x00623e20), GP4_GLOBAL(int32_t, 0x00623e78))) + partial;
    }
    // DAT_00623e28 = e18*e64 + e1c*e70 + e20*e7c
    {
        const uint32_t partial = shifted(product(GP4_GLOBAL(int32_t, 0x00623e18), GP4_GLOBAL(int32_t, 0x00623e64)))
            + shifted(product(GP4_GLOBAL(int32_t, 0x00623e1c), GP4_GLOBAL(int32_t, 0x00623e70)));
        GP4_GLOBAL(uint32_t, 0x00623e28) =
            shifted(product(GP4_GLOBAL(int32_t, 0x00623e20), GP4_GLOBAL(int32_t, 0x00623e7c))) + partial;
    }
    // DAT_00623e2c = e18*e68 + e1c*e74 + e20*e80; the original leaves the last
    // sum in eax, the partial sum in ebp, the last product's high half in edx
    // and the flags of the final "add eax, ebp".
    const uint32_t partial = shifted(product(GP4_GLOBAL(int32_t, 0x00623e18), GP4_GLOBAL(int32_t, 0x00623e68)))
        + shifted(product(GP4_GLOBAL(int32_t, 0x00623e1c), GP4_GLOBAL(int32_t, 0x00623e74)));
    const int64_t lastProduct = product(GP4_GLOBAL(int32_t, 0x00623e20), GP4_GLOBAL(int32_t, 0x00623e80));
    const uint32_t last = shifted(lastProduct);
    const uint32_t sum = last + partial;
    GP4_GLOBAL(uint32_t, 0x00623e2c) = sum;

    r->eax = sum;
    r->ebp = partial;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(lastProduct) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(last, partial);
}
GP4_IMPL(0x00401adc, FUN_00401adc, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
