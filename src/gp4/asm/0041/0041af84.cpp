#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 8-bit `left - right` (both operands already in 0..255).
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80u) << 4);
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

struct Unknown_0041af84 {
    uint8_t Unknown00[0x72];
    int8_t Level72;
    uint8_t Unknown73[0x9a - 0x73];
    int16_t Value9A;
    uint8_t Unknown9C[0x1c6 - 0x9c];
    uint8_t Table1C6[1];
};

// CF = 1 (stc) when the byte at +0x72 is below 2, or when ((table value of level-1) * word(+0x9a)
// >> 14) is at least DAT_00601320 (signed); otherwise CF = 0. The scaled product is left in eax
// and its high half in edx; the other flags are those of the final cmp (or of cmp al, 2).
// FUNCTION: GP4 0x0041af84
void __cdecl FUN_0041af84(gp4::Regs* r) {
    const Unknown_0041af84* body = reinterpret_cast<const Unknown_0041af84*>(static_cast<uintptr_t>(r->esi));
    const uint32_t level = static_cast<uint8_t>(body->Level72);
    if (static_cast<int8_t>(body->Level72) < 2) {
        r->eax = (r->eax & 0xffffff00u) | level;
        r->eflags = (r->eflags & ~0x8d5u) | subFlags8(level, 2u) | 1u;
        return;
    }

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.eax = (r->eax & 0xffffff00u) | ((level - 1u) & 0xffu);
    callee.edx = r->esi + 0x1c6u;
    gp4::call_regs(0x0041242f, callee);

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(callee.eax))
        * static_cast<int64_t>(body->Value9A);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t scaled = (low >> 14) | (high << 18);

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00601320);
    const bool below = static_cast<int32_t>(scaled) < static_cast<int32_t>(limit);
    const uint32_t flags = (subFlags32(scaled, limit) & ~1u) | (below ? 0u : 1u);

    r->eax = scaled;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041af84, FUN_0041af84, "regs(eax:u32, esi:ptr[0x250]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
