#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Clamps the index in eax to 0..21 and returns the address of that 0x41c-byte record of the
// table at 0x0066fef0. The flags left behind are those of the final "add eax, 0x66fef0".
// FUNCTION: GP4 0x0040838c
void __cdecl FUN_0040838c(gp4::Regs* r) {
    int32_t index = static_cast<int32_t>(r->eax);
    if (index < 0) index = 0;
    if (index > 0x15) index = 0x15;

    const uint32_t offset = static_cast<uint32_t>(index) * 0x41cu;
    const uint32_t base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x0066fef0)));
    const uint32_t result = offset + base;

    const uint32_t flags = (result < offset ? 0x1u : 0u)
        | parityFlag(result)
        | (((offset ^ base ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((offset ^ result) & (base ^ result)) >> 31) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040838c, FUN_0040838c, "regs(eax:i32[-8..40]) -> (eax, cf, pf, af, zf, sf, of)")
