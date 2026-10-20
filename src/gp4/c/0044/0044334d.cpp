#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t cmp32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Time since the current car's +0xc0 stamp ([0x679d30] car, 0xc0000000 = "no previous time"), split
// into decimal digits by 0x0040996f, clamped to 0x40000 and stored shifted left by 12 at [0x66d860].
// eax and edx are preserved (push/pop); the flags are those of `cmp [car+0xc0], 0xc0000000` with CF
// forced: set (stc) unless the stamp is the sentinel, cleared (clc) when it is.
// FUNCTION: GP4 0x0044334d
void __cdecl FUN_0044334d(gp4::Regs* r) {
    uint8_t* car = GP4_GLOBAL(uint8_t*, 0x00679d30);

    gp4::Regs digits = *r;
    digits.eax = GP4_GLOBAL(uint32_t, 0x0066d814) - GP4_FIELD(uint32_t, car, 0xc0);
    gp4::call_regs(0x0040996f, digits);

    int32_t value = static_cast<int32_t>(digits.eax);
    if (value > 0x40000) {
        value = 0x40000;
    }
    GP4_GLOBAL(uint32_t, 0x0066d860) = static_cast<uint32_t>(value) << 12;

    car = GP4_GLOBAL(uint8_t*, 0x00679d30);
    const uint32_t stamp = GP4_FIELD(uint32_t, car, 0xc0);
    const uint32_t flags = cmp32Flags(stamp, 0xc0000000u);
    const uint32_t carry = (stamp != 0xc0000000u) ? 1u : 0u;

    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x0044334d, FUN_0044334d, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00418f8f+state_0044334d")
