#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043cdcc
void __cdecl FUN_0043cdcc(gp4::Regs* r) {
    // edx:eax = word[0x62d210 + index*2] * 0x83127 (signed), then shrd eax, edx, 14; edx is restored.
    const uint32_t entry = GP4_ARRAY(uint16_t, 0x0062d210)[r->eax];
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(entry)) * 0x83127);
    const uint32_t result = static_cast<uint32_t>(product >> 14);
    const uint32_t previous = static_cast<uint32_t>(product >> 13);

    // shrd by 14 (pinned oracle model): CF = last bit shifted out, AF = 0,
    // OF = bit 31 of (value shifted by 13) xor (value shifted by 14).
    const uint32_t flags = parityFlag(result)
        | (((product >> 13) & 1u) != 0u ? 1u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) & 0x80000000u) != 0u) ? 0x800u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043cdcc, FUN_0043cdcc, "regs(eax:u32[0..4]) -> (eax, cf, pf, af, zf, sf, of)")
