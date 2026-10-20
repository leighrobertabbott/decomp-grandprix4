#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit cmp left, right, without CF (the routine sets CF with clc/stc).
static inline uint32_t cmp8FlagsNoCarry(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040822c
void __cdecl FUN_0040822c(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t value = static_cast<int8_t>(record[0xe4]);
    uint32_t flags;
    if ((record[0xe5] & 4u) != 0u) {
        // test [esi+0xe5], 4 -> result 4: PF = 0 (one bit set), ZF = SF = 0; clc.
        flags = 0u;
    } else if (value < 3) {
        // cmp [esi+0xe4], 3 ; jl -> clc
        flags = cmp8FlagsNoCarry(record[0xe4], 3u);
    } else if (value > 4) {
        // cmp [esi+0xe4], 4 ; jg -> clc
        flags = cmp8FlagsNoCarry(record[0xe4], 4u);
    } else {
        // stc: carry set, other flags from cmp [esi+0xe4], 4
        flags = cmp8FlagsNoCarry(record[0xe4], 4u) | 1u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040822c, FUN_0040822c, "regs(esi:ptr[0xed]:bytes) -> (cf, pf, af, zf, sf, of)")
