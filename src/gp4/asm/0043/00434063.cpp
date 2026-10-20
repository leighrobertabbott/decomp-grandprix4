#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `left - right` (cmp / sub).
static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Sets bit `bit` of the 64-bit set stored at slot `slot` (8-byte stride) in two parallel tables.
// bts with a register bit offset addresses dword (bit >> 5) relative to the operand.
// Returns CF of the second bts (its previous bit value).
static inline uint32_t setBitBoth(uint32_t baseA, uint32_t baseB, uint32_t slot, int32_t bit) {
    uint32_t* wordA = &GP4_ARRAY(uint32_t, baseA)[slot * 2u + static_cast<uint32_t>(bit >> 5)];
    uint32_t* wordB = &GP4_ARRAY(uint32_t, baseB)[slot * 2u + static_cast<uint32_t>(bit >> 5)];
    const uint32_t mask = 1u << (bit & 31);
    *wordA |= mask;
    const uint32_t previous = (*wordB & mask) != 0u ? 1u : 0u;
    *wordB |= mask;
    return previous;
}

// Marks bit `edx` (0..63) of slot `eax` in two 64-bit bit sets: the low dword pair when the bit is
// below 0x20, the high pair (bit - 0x20) otherwise. Preserves edx; leaves the flags of the
// cmp/sub with CF replaced by the last bts.
// FUNCTION: GP4 0x00434063
void __cdecl FUN_00434063(gp4::Regs* r) {
    const int32_t bit = static_cast<int32_t>(r->edx);
    const uint32_t slot = r->eax;
    uint32_t flags;
    uint32_t previous;

    if (bit < 0x20) {
        flags = subtractionFlags32(r->edx, 0x20u);                       // cmp edx, 0x20
        previous = setBitBoth(0x0062fc18u, 0x0063b94cu, slot, bit);
    } else {
        const int32_t high = bit - 0x20;
        flags = subtractionFlags32(r->edx, 0x20u);                       // sub edx, 0x20
        previous = setBitBoth(0x0062fc1cu, 0x0063b950u, slot, high);
    }

    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | previous;
}
GP4_IMPL(0x00434063, FUN_00434063, "regs(edx:i32[0..63], eax:u32[0..31]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
