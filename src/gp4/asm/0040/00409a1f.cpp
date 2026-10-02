#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00409a1f
void __cdecl FUN_00409a1f(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t input = r->eax;
    // imul eax, eax, 0x5dde ; sar eax, 8 ; only ax is compared
    const uint32_t product = input * 0x5ddeu;
    const uint16_t left = static_cast<uint16_t>(static_cast<int32_t>(product) >> 8);
    const uint16_t right = GP4_FIELD(uint16_t, record, 0x9a);
    // cmp ax, [esi+0x9a]
    const uint16_t result = static_cast<uint16_t>(left - right);
    const bool sign = (result & 0x8000u) != 0u;
    const bool overflow = (((left ^ right) & (left ^ result) & 0x8000u) != 0u);
    // jl: stc, otherwise clc (pop eax restores eax and leaves the flags alone)
    const uint32_t flags = (sign != overflow ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (sign ? 0x80u : 0u)
        | (overflow ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00409a1f, FUN_00409a1f, "regs(eax:u32, esi:ptr[0xa2]:bytes) -> (cf, pf, af, zf, sf, of)")
