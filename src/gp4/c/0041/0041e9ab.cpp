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

// 32x32 -> 64 signed multiply followed by "shrd eax, edx, 14".
static inline uint32_t mulShift14(uint32_t left, int32_t right) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(left)) * static_cast<int64_t>(right);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return (low >> 14) | (high << 18);
}

// FUNCTION: GP4 0x0041e9ab
void __cdecl FUN_0041e9ab(gp4::Regs* r) {
    const int32_t first = GP4_GLOBAL(int32_t, 0x00638000);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00637fec);
    const uint32_t second = GP4_GLOBAL(uint32_t, 0x00638004);
    const uint32_t third = GP4_GLOBAL(uint32_t, 0x00638008);

    uint32_t value = mulShift14(static_cast<uint32_t>(first) * 3u, scale);
    value += second;
    value += second;
    value = mulShift14(value, scale);
    const uint32_t flags = additionFlags32(value, third);
    value += third;

    const int32_t dividend = static_cast<int32_t>(value);
    GP4_GLOBAL(int32_t, 0x00638010) = dividend / 6;
    r->eax = static_cast<uint32_t>(dividend / 6);
    r->edx = static_cast<uint32_t>(dividend % 6);
    r->ebp = 6;
    // idiv leaves the flags of the preceding add.
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041e9ab, FUN_0041e9ab, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
