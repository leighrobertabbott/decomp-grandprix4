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

// FUNCTION: GP4 0x0041a099
void __cdecl FUN_0041a099(gp4::Regs* r) {
    const int32_t scale = GP4_GLOBAL(int32_t, 0x0060384c);

    GP4_GLOBAL(int32_t, 0x0063804c) += static_cast<int32_t>(static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00637f60)) * scale >> 8);

    const uint32_t oldSecond = GP4_GLOBAL(uint32_t, 0x00638054);
    const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00637f64)) * scale;
    const uint32_t delta = static_cast<uint32_t>(product >> 8);
    GP4_GLOBAL(uint32_t, 0x00638054) = oldSecond + delta;

    r->ebp = oldSecond;
    r->eax = delta;
    r->edx = static_cast<uint32_t>(product >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(oldSecond, delta);
}
GP4_IMPL(0x0041a099, FUN_0041a099, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
