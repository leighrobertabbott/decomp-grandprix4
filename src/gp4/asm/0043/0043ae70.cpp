#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 8-bit `left - right` (cmp byte ptr [..], imm8).
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) << 4);
}

// Flags of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// 0x78-byte records at 0x0063bd7c (only the field this routine reads).
struct Unknown_0063bd7c {
    uint8_t Unknown00[0x6c];
    uint8_t Unknown6C;
    uint8_t Unknown6D[0x78 - 0x6d];
};

// Returns the 0x41c-byte body at 0x0066fef0 selected by byte +0x6c of record eax (0x78-byte
// records at 0x0063bd7c) when the active flag byte [0x0063bcfc + eax] equals 1, else 0.
// The flags are those of the cmp (no match) or of the final "add eax, 0x66fef0".
// FUNCTION: GP4 0x0043ae70
void __cdecl FUN_0043ae70(gp4::Regs* r) {
    const uint32_t index = r->eax;
    const uint32_t active = GP4_ARRAY(uint8_t, 0x0063bcfc)[index];
    uint32_t flags = cmpFlags8(active, 1u);
    uint32_t result = 0u;

    if (active == 1u) {
        const Unknown_0063bd7c* rec = GP4_ARRAY(Unknown_0063bd7c, 0x0063bd7c) + index;
        const uint32_t offset = static_cast<uint32_t>(rec->Unknown6C) * 0x41cu;
        result = offset + 0x0066fef0u;
        flags = addFlags(offset, 0x0066fef0u, result);
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ae70, FUN_0043ae70, "regs(eax:u32[0..63]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=bytes_0063bcfc")
