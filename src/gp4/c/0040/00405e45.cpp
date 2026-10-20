#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

struct Unknown_00405e45 {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint16_t Unknown62;
};

// Distance of the position computed by 0x403800 (edi) from a window edge of the record at edi:
// sign bit set in DAT_00868582: eax = position - (word(+0x60) + 0x50); otherwise
// eax = (word(+0x62) + 0x50) - position. edx is saved and restored; the flags are those of the
// final add / sub.
// FUNCTION: GP4 0x00405e45
void __cdecl FUN_00405e45(gp4::Regs* r) {
    const Unknown_00405e45* rec = reinterpret_cast<const Unknown_00405e45*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x00403800, callee);
    const uint32_t position = callee.eax;

    uint32_t result;
    uint32_t flags;
    if ((GP4_GLOBAL(uint8_t, 0x00868582) & 0x80u) != 0u) {
        const uint32_t negated = 0u - (static_cast<uint32_t>(rec->Unknown60) + 0x50u);
        result = negated + position;
        flags = addFlags(negated, position, result);
    } else {
        const uint32_t edge = static_cast<uint32_t>(rec->Unknown62) + 0x50u;
        result = edge - position;
        flags = subFlags(edge, position);
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00405e45, FUN_00405e45, "regs(edi:ptr[0x70]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
