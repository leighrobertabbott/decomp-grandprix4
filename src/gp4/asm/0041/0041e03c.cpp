#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right` (also `neg x` as 0 - x).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

struct Unknown_0041e03c {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint16_t Unknown62;
};

// Clamps a signed value in eax to the bias (word [edi+0x60 or +0x62] + 0x50) << 8, using the first
// word for the magnitude of a negative value (which keeps its sign). edx is saved and restored; the
// flags are those of the last neg (negative input) or of the cmp (non-negative input).
// FUNCTION: GP4 0x0041e03c
void __cdecl FUN_0041e03c(gp4::Regs* r) {
    const Unknown_0041e03c* record = reinterpret_cast<const Unknown_0041e03c*>(static_cast<uintptr_t>(r->edi));
    uint32_t value = r->eax;
    uint32_t flags;
    if (static_cast<int32_t>(value) < 0) {
        value = 0u - value;
        const uint32_t bias = (static_cast<uint32_t>(record->Unknown60) + 0x50u) << 8;
        if (static_cast<int32_t>(value) > static_cast<int32_t>(bias)) {
            value = bias;
        }
        flags = subFlags32(0u, value);
        value = 0u - value;
    } else {
        const uint32_t bias = (static_cast<uint32_t>(record->Unknown62) + 0x50u) << 8;
        flags = subFlags32(value, bias);
        if (static_cast<int32_t>(value) > static_cast<int32_t>(bias)) {
            value = bias;
        }
    }
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041e03c, FUN_0041e03c, "regs(eax:i32, edi:ptr[0x6c]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
