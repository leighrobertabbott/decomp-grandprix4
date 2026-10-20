#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// (DAT_0062474c - DAT_006258ac) << 19 divided by DAT_006013a8 (idiv), scaled by DAT_0060384c
// (shrd 16) and DAT_00601e84 (shrd 20). edx keeps the high half of the last product; the flags are
// those of the final shrd under the pinned oracle model (CF = bit 19 of the product, AF = 0,
// OF = bit 31 of the product shifted by 19 xor the product shifted by 20).
// FUNCTION: GP4 0x0041bd28
void __cdecl FUN_0041bd28(gp4::Regs* r) {
    const int32_t difference = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062474c) - GP4_GLOBAL(uint32_t, 0x006258ac));
    const int64_t dividend = static_cast<int64_t>(static_cast<uint64_t>(static_cast<int64_t>(difference)) << 19);

    int32_t remainder;
    const int32_t quotient = gp4::idiv64(dividend, GP4_GLOBAL(int32_t, 0x006013a8), &remainder);

    const int64_t first = static_cast<int64_t>(quotient) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060384c));
    const int32_t shifted = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(first) >> 16));

    const uint64_t bits = static_cast<uint64_t>(static_cast<int64_t>(shifted) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601e84)));
    const uint32_t result = static_cast<uint32_t>(bits >> 20);

    const uint32_t flags = static_cast<uint32_t>((bits >> 19) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((bits >> 50) ^ (bits >> 51)) & 1u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->edx = static_cast<uint32_t>(bits >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041bd28, FUN_0041bd28, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0041bd28")
