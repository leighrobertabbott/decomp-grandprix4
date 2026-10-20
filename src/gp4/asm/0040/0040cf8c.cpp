#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right` (also `neg x` as 0 - x).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Flags of a dword-wide logic result (or edx, edx): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// Signed distance (edx - eax) with the dead zone DAT_00866ec4 removed beyond DAT_00866ed8,
// divided by 0x160 (quotient in eax, remainder in edx). The flags are those of the last or / neg
// before the idiv, which leaves them untouched; ebp is saved and restored.
// FUNCTION: GP4 0x0040cf8c
void __cdecl FUN_0040cf8c(gp4::Regs* r) {
    const uint32_t difference = r->edx - r->eax;
    uint32_t magnitude = difference;
    if (static_cast<int32_t>(difference) < 0) {
        magnitude = 0u - magnitude;
    }

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00866ed8);
    if (static_cast<int32_t>(magnitude) >= static_cast<int32_t>(limit)) {
        magnitude -= GP4_GLOBAL(uint32_t, 0x00866ec4);
    }

    uint32_t flags = logicFlags32(difference);
    if (static_cast<int32_t>(difference) < 0) {
        flags = subFlags32(0u, magnitude);
        magnitude = 0u - magnitude;
    }

    int32_t remainder;
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(magnitude)), 0x160, &remainder);

    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040cf8c, FUN_0040cf8c, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
