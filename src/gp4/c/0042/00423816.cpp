#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// 64-bit accumulator {lo, hi} += sign-extended 32-bit value (cdq; add lo, eax; adc hi, edx).
// Returns the flags of the final adc.
static inline uint32_t addSigned(uint32_t& lo, uint32_t& hi, uint32_t value) {
    const uint32_t extension = (value & 0x80000000u) != 0u ? 0xffffffffu : 0u;
    const uint32_t newLo = lo + value;
    const uint32_t carryIn = newLo < lo ? 1u : 0u;
    lo = newLo;

    const uint32_t before = hi;
    const uint32_t result = before + extension + carryIn;
    hi = result;
    const bool carryOut = carryIn != 0u ? (result <= before) : (result < before);
    return parityFlag(result)
        | (((before ^ extension ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(before ^ extension) & (before ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (carryOut ? 1u : 0u);
}

// FUNCTION: GP4 0x00423816
// Six 64-bit accumulators at 0x00607ad0.. each gain a sign-extended 32-bit delta.
void __cdecl FUN_00423816(gp4::Regs* r) {
    addSigned(GP4_GLOBAL(uint32_t, 0x00607ad0), GP4_GLOBAL(uint32_t, 0x00607ad4), GP4_GLOBAL(uint32_t, 0x00607ff4));
    addSigned(GP4_GLOBAL(uint32_t, 0x00607ad8), GP4_GLOBAL(uint32_t, 0x00607adc), GP4_GLOBAL(uint32_t, 0x00607ff8));
    addSigned(GP4_GLOBAL(uint32_t, 0x00607ae0), GP4_GLOBAL(uint32_t, 0x00607ae4), GP4_GLOBAL(uint32_t, 0x00607ffc));
    addSigned(GP4_GLOBAL(uint32_t, 0x00607ae8), GP4_GLOBAL(uint32_t, 0x00607aec), GP4_GLOBAL(uint32_t, 0x006081d4));
    addSigned(GP4_GLOBAL(uint32_t, 0x00607af0), GP4_GLOBAL(uint32_t, 0x00607af4), GP4_GLOBAL(uint32_t, 0x006081d8));
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x006081dc);
    const uint32_t flags = addSigned(GP4_GLOBAL(uint32_t, 0x00607af8), GP4_GLOBAL(uint32_t, 0x00607afc), last);

    r->eax = last;
    r->edx = (last & 0x80000000u) != 0u ? 0xffffffffu : 0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00423816, FUN_00423816, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
