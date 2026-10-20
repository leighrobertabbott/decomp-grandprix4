#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// base of the 4-column table that the original addresses through ebx
static __declspec(noinline) const uint32_t* tableBase() {
    return reinterpret_cast<const uint32_t*>(GP4_ARRAY(uint8_t, 0x006380d8));
}

// FUNCTION: GP4 0x00419f9e
void __cdecl FUN_00419f9e(gp4::Regs* r) {
    const uint32_t* t = tableBase();
    GP4_GLOBAL(uint32_t, 0x00637fc8) = t[0x634 / 4] + t[0x934 / 4] + t[0x34 / 4] + t[0x334 / 4]
        + GP4_GLOBAL(uint32_t, 0x00601fec) + GP4_GLOBAL(uint32_t, 0x00601fe8)
        + GP4_GLOBAL(uint32_t, 0x00637fa4) + GP4_GLOBAL(uint32_t, 0x00637fbc)
        + GP4_GLOBAL(uint32_t, 0x00637fb0);

    const uint32_t partial = t[0x3c / 4] + t[0x33c / 4] + t[0x63c / 4] + t[0x93c / 4]
        + GP4_GLOBAL(uint32_t, 0x00601ff4) + GP4_GLOBAL(uint32_t, 0x00601ff0)
        + GP4_GLOBAL(uint32_t, 0x00637fc4) + GP4_GLOBAL(uint32_t, 0x00637fb8)
        + GP4_GLOBAL(uint32_t, 0x00637fac);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00601ac0);
    const uint32_t total = partial + last;
    GP4_GLOBAL(uint32_t, 0x00637fd0) = total;

    // ebx keeps the table base; eax the second sum; the flags are those of the final add
    r->ebx = 0x006380d8u;
    r->eax = total;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(partial, last);
}
GP4_IMPL(0x00419f9e, FUN_00419f9e, "regs() -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sums_00419f9e")
