#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Arithmetic flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) << 4);
}

// FUNCTION: GP4 0x00446121
// Selector byte [0x7a8dff] == 0xff forces the mask byte [0x7c2ead] to 0x40 (al is left 0x40).
// Otherwise, when [0x7a8df8] == 0xff and bit 6 of [0x62f8fe] & [0x7c2ead] is set, the byte
// [0x62f8ff] becomes 0xff (al is left holding the AND). The flags are those of the last
// cmp / or / test.
void __cdecl FUN_00446121(gp4::Regs* r) {
    uint32_t flags;
    if (GP4_GLOBAL(uint8_t, 0x007a8dff) == 0xffu) {
        r->eax = (r->eax & 0xffffff00u) | 0x40u;
        GP4_GLOBAL(uint8_t, 0x007c2ead) = 0x40u;
        flags = logicFlags8(0x40u);
    } else {
        const uint32_t pending = GP4_GLOBAL(uint8_t, 0x007a8df8);
        flags = cmpFlags8(pending, 0xffu);
        if (pending == 0xffu) {
            const uint32_t masked = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x0062f8fe)) & GP4_GLOBAL(uint8_t, 0x007c2ead);
            r->eax = (r->eax & 0xffffff00u) | masked;
            flags = logicFlags8(masked & 0x40u);
            if ((masked & 0x40u) != 0u) {
                GP4_GLOBAL(uint8_t, 0x0062f8ff) = 0xffu;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00446121, FUN_00446121, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
