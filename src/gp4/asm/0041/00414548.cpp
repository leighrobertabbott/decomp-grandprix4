#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00414548
void __cdecl FUN_00414548(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t left = GP4_FIELD(uint8_t, record, 0x235);
    const uint32_t right = 2u;
    const uint32_t result = (left - right) & 0xffu;
    // cmp byte [esi+0x235], 2, then stc (>=) or clc (<) rewrites CF
    const uint32_t flags = parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u)
        | (left >= right ? 1u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00414548, FUN_00414548, "regs(esi:ptr[0x23d]:bytes) -> (cf, pf, af, zf, sf, of)")
