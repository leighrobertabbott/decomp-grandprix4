#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Piecewise response of the signed value in eax: below DAT_00601314 it is returned unchanged;
// above it, (eax - DAT_00601314) * DAT_00603a7c >> 14 is added back to DAT_00601314 and the sum is
// capped at DAT_00601314 + 0x190. edx is saved and restored; the flags are those of the last cmp.
// FUNCTION: GP4 0x00415baf
void __cdecl FUN_00415baf(gp4::Regs* r) {
    const uint32_t value = r->eax;
    const uint32_t knee = GP4_GLOBAL(uint32_t, 0x00601314);
    uint32_t flags = subFlags32(value, knee);
    if (static_cast<int32_t>(value) >= static_cast<int32_t>(knee)) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(value - knee))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603a7c));
        uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + knee;
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00601314) + 0x190u;
        flags = subFlags32(result, limit);
        if (static_cast<int32_t>(result) > static_cast<int32_t>(limit)) {
            result = limit;
        }
        r->eax = result;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00415baf, FUN_00415baf, "regs(eax:i32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
