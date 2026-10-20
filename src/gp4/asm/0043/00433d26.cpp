#include <gp4/gp4.h>

struct Unknown_00433d26 {
    uint8_t Unknown00[0xe4];
    int8_t UnknownE4;
    uint8_t UnknownE5[0xb];
    int32_t UnknownF0;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 8-bit `left - right` (cmp byte ptr, imm8).
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) << 4);
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// For a record whose signed state byte at +0xe4 is in 3..5, returns the word at +0xf0 minus
// DAT_0066d814; otherwise 0. The flags are those of the last comparison / subtraction executed.
// FUNCTION: GP4 0x00433d26
void __cdecl FUN_00433d26(gp4::Regs* r) {
    const Unknown_00433d26* rec = reinterpret_cast<const Unknown_00433d26*>(static_cast<uintptr_t>(r->esi));
    const uint32_t state = static_cast<uint32_t>(static_cast<uint8_t>(rec->UnknownE4));

    uint32_t result;
    uint32_t flags;
    if (rec->UnknownE4 < 3) {
        result = 0u;
        flags = cmpFlags8(state, 3u);
    } else if (rec->UnknownE4 > 5) {
        result = 0u;
        flags = cmpFlags8(state, 5u);
    } else {
        const uint32_t value = static_cast<uint32_t>(rec->UnknownF0);
        const uint32_t base = GP4_GLOBAL(uint32_t, 0x0066d814);
        result = value - base;
        flags = subFlags(value, base);
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00433d26, FUN_00433d26, "regs(esi:ptr[0xf8]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
