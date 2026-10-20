#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Removes a bias from a signed value in eax: the bias is (word [edi+0x60 or +0x62] + 0x50) << 8,
// the first word being used (and the value negated first) when eax is negative. edx is preserved;
// the flags are those of the final "sub eax, edx".
// FUNCTION: GP4 0x0041e019
void __cdecl FUN_0041e019(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t value = r->eax;
    uint32_t bias;
    if (static_cast<int32_t>(value) < 0) {
        value = 0u - value;
        bias = (GP4_FIELD(uint16_t, record, 0x60) + 0x50u) << 8;
    } else {
        bias = (GP4_FIELD(uint16_t, record, 0x62) + 0x50u) << 8;
    }
    const uint32_t result = value - bias;

    const uint32_t flags = (value < bias ? 0x1u : 0u)
        | parityFlag(result)
        | (((value ^ bias ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((value ^ bias) & (value ^ result)) >> 31) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041e019, FUN_0041e019, "regs(eax:i32, edi:ptr[0x6c]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
