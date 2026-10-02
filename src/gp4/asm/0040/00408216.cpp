#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit cmp left, right, without CF (the caller sets CF with clc/stc).
static inline uint32_t cmp8FlagsNoCarry(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00408216
void __cdecl FUN_00408216(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t value = static_cast<int8_t>(record[0xe4]);
    uint32_t flags;
    if (value < 2) {
        // cmp [esi+0xe4], 2 ; jl -> clc
        flags = cmp8FlagsNoCarry(record[0xe4], 2u);
    } else {
        // cmp [esi+0xe4], 5 ; jle -> stc, otherwise clc
        flags = cmp8FlagsNoCarry(record[0xe4], 5u) | (value <= 5 ? 1u : 0u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408216, FUN_00408216, "regs(esi:ptr[0xec]:bytes) -> (cf, pf, af, zf, sf, of)")
