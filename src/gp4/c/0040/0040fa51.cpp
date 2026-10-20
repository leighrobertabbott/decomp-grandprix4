#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040fa51
// Runs 0x0040fa6f over the 22 records of stride 0x41c starting at 0x0066fef0.
// eax/ecx/esi are saved and restored; the flags come from the last "add esi, 0x41c".
void __cdecl FUN_0040fa51(gp4::Regs* r) {
    gp4::Regs saved = *r;

    uint32_t record = 0x0066fef0u;
    uint32_t flags = 0u;
    for (uint32_t remaining = 0x16u; remaining != 0u; remaining--) {
        gp4::Regs call = saved;
        call.esi = record;
        gp4::call_regs(0x0040fa6f, call);
        flags = additionFlags32(record, 0x41cu);
        record += 0x41cu;
    }

    saved.eflags = (saved.eflags & ~0x8d5u) | flags;
    *r = saved;
}
GP4_IMPL(0x0040fa51, FUN_0040fa51, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=cars_0040fa51; cases=200")
