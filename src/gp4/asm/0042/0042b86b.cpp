#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of the 16-bit "cmp lhs, rhs"
static inline uint32_t cmpFlags16(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = (lhs - rhs) & 0xffffu;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 8) & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// flags of the 32-bit "cmp lhs, rhs"
static inline uint32_t cmpFlags32(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = lhs - rhs;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// When the word at esi+0xea is 8 and ebx equals the dword at esi+0x114, that word becomes 0xfff8 (-8).
// dx is loaded with the old word; the flags are those of the last compare.
// FUNCTION: GP4 0x0042b86b
void __cdecl FUN_0042b86b(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t word = GP4_FIELD(uint16_t, record, 0xea);
    r->edx = (r->edx & 0xffff0000u) | word;
    uint32_t flags = cmpFlags16(word, 8u);
    if (word == 8u) {
        const uint32_t other = GP4_FIELD(uint32_t, record, 0x114);
        flags = cmpFlags32(r->ebx, other);
        if (r->ebx == other) {
            GP4_FIELD(uint16_t, record, 0xea) = 0xfff8u;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042b86b, FUN_0042b86b, "regs(esi:ptr[0x11c]:layout=rec_0042b84b, ebx:u32[0..3], edx:u32) -> (edx, cf, pf, af, zf, sf, of)")
