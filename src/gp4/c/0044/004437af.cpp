#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Fixed-point accumulate: [0x0062ad6c] += ([0x0062ad68] * [0x00603854]) >> 8 (the 64-bit product shifted right by 8,
// low 32 bits kept). eax receives the new sum, edx the high half of the product; the flags are those of the add.
// FUNCTION: GP4 0x004437af
void __cdecl FUN_004437af(gp4::Regs* r) {
    const uint64_t product = static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x0062ad68)) * static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x00603854));
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(product >> 32);
    const uint32_t shifted = (low >> 8) | (high << 24);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062ad6c);
    const uint32_t result = shifted + before;
    GP4_GLOBAL(uint32_t, 0x0062ad6c) = result;

    r->eax = result;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(shifted, before);
}
GP4_IMPL(0x004437af, FUN_004437af, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
