#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004354e7
void __cdecl FUN_004354e7(gp4::Regs* r) {
    const uint32_t packed = r->eax;
    // sum of the four bytes of eax (at most 4 * 255)
    const uint32_t sum = (packed & 0xffu) + ((packed >> 8) & 0xffu) + ((packed >> 16) & 0xffu) + (packed >> 24);
    const uint32_t quotient = (sum << 14) / 0x2c0u;

    // cmp eax, 0x4000 leaves its flags; the clamp's mov does not touch them
    const uint32_t limit = 0x4000u;
    const uint32_t result = quotient - limit;
    const uint32_t flags = parityFlag(result)
        | (((quotient ^ limit ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((quotient ^ limit) & (quotient ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (quotient < limit ? 1u : 0u);

    r->eax = (static_cast<int32_t>(quotient) > 0x4000) ? 0x4000u : quotient;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004354e7, FUN_004354e7, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
