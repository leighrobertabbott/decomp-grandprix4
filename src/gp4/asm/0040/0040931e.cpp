#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right" on a dword
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x0040931e
// Clamps the signed value in eax to +-((word [edi + 0x62 / 0x60] + 0x50) << 8): a negative value is
// raised to the negated limit from +0x60, a non-negative one lowered to the limit from +0x62.
// edx is preserved; the flags are those of the final "cmp eax, limit".
void __cdecl FUN_0040931e(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const int32_t value = (int32_t)r->eax;
    int32_t limit;
    int32_t result = value;

    if (value < 0) {
        limit = -(int32_t)(((uint32_t)GP4_FIELD(uint16_t, body, 0x60) + 0x50u) << 8);
        if (value < limit) {
            result = limit;
        }
    } else {
        limit = (int32_t)(((uint32_t)GP4_FIELD(uint16_t, body, 0x62) + 0x50u) << 8);
        if (value > limit) {
            result = limit;
        }
    }

    r->eax = (uint32_t)result;
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32((uint32_t)value, (uint32_t)limit);
}
GP4_IMPL(0x0040931e, FUN_0040931e, "regs(eax:i32[-20000000..20000000], edi:ptr[0x68]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
