#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of the 32-bit "cmp lhs, rhs"
static inline uint32_t cmpFlags32(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = lhs - rhs;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// Stores eax clamped to 0..11 in DAT_00622584 (-1 is stored unchanged); eax itself is preserved.
// The flags left behind are those of the last compare executed.
// FUNCTION: GP4 0x0040c3a7
void __cdecl FUN_0040c3a7(gp4::Regs* r) {
    int32_t value = static_cast<int32_t>(r->eax);
    uint32_t flags;
    if (value == -1) {
        flags = cmpFlags32(static_cast<uint32_t>(value), 0xffffffffu);
    } else {
        if (value < 0) {
            value = 0;
        }
        flags = cmpFlags32(static_cast<uint32_t>(value), 0xbu);
        if (value > 0xb) {
            value = 0xb;
        }
    }
    GP4_GLOBAL(int32_t, 0x00622584) = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040c3a7, FUN_0040c3a7, "regs(eax:i32[-4..20]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
