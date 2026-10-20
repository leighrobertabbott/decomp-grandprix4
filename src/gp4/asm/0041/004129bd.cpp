#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
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

// flags of the 8-bit "cmp lhs, rhs"
static inline uint32_t cmpFlags8(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = (lhs - rhs) & 0xffu;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Stores a state byte at esi+0x294: 1 when the dword at esi+0x1c is not below DAT_00891500,
// otherwise DAT_00601ba5 (with 1 mapped to 7). AL carries it out; edi is preserved. The flags
// are those of the last compare executed.
// FUNCTION: GP4 0x004129bd
void __cdecl FUN_004129bd(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t value = GP4_FIELD(uint32_t, record, 0x1c);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    uint32_t state;
    uint32_t flags;
    if (value >= limit) {
        state = 1u;
        flags = cmpFlags32(value, limit);
    } else {
        state = GP4_GLOBAL(uint8_t, 0x00601ba5);
        flags = cmpFlags8(state, 1u);
        if (state == 1u) {
            state = 7u;
        }
    }
    GP4_FIELD(uint8_t, record, 0x294) = static_cast<uint8_t>(state);
    r->eax = (r->eax & 0xffffff00u) | state;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004129bd, FUN_004129bd, "regs(esi:ptr[0x29c]:bytes, eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
