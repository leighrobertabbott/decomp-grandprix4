#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t AddFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return (result < lhs ? 1u : 0u) | parityFlag(result) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// Interpolates between DAT_0062279c (start) and DAT_00622798 (end) by the 6-bit-scaled table
// byte at 0x0066fd20[eax]: eax = ((byte << 6) * (end - start) >> 14) + start. edx is preserved.
// The flags are those of the final "add eax, start".
// FUNCTION: GP4 0x0043ca66
void __cdecl FUN_0043ca66(gp4::Regs* r) {
    const uint32_t index = r->eax;
    const uint32_t scaled = static_cast<uint32_t>(GP4_ARRAY(uint8_t, 0x0066fd20)[index]) << 6;
    const uint32_t end = GP4_GLOBAL(uint32_t, 0x00622798);
    const uint32_t start = GP4_GLOBAL(uint32_t, 0x0062279c);

    // imul edx ; shrd eax, edx, 14
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(scaled))
        * static_cast<int64_t>(static_cast<int32_t>(end - start));
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t result = shifted + start;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | AddFlags(shifted, start, result);
}
GP4_IMPL(0x0043ca66, FUN_0043ca66, "regs(eax:u32[0..21]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
