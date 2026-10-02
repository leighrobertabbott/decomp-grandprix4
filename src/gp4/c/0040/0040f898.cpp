#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `cmp left, right` on 32 bits.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of `test byte, imm8` with the mask already applied: CF=OF=AF=0.
static inline uint32_t testFlags8(uint8_t result) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0040f898
void __cdecl FUN_0040f898(gp4::Regs* r) {
    uint8_t* root = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x00679d30));
    const uint32_t other = GP4_GLOBAL(uint32_t, 0x00679d2c);
    uint32_t flags = cmpFlags32(reinterpret_cast<uint32_t>(root), other);
    r->esi = reinterpret_cast<uint32_t>(root);

    if (reinterpret_cast<uint32_t>(root) == other) {
        uint8_t* child = reinterpret_cast<uint8_t*>(GP4_FIELD(uint32_t, root, 0x1c));
        r->edi = reinterpret_cast<uint32_t>(child);
        const uint8_t masked = static_cast<uint8_t>(GP4_FIELD(uint8_t, child, 0x51) & 0x80u);
        flags = testFlags8(masked);
        if (masked == 0u) {
            const uint32_t diff = static_cast<uint32_t>(GP4_FIELD(uint16_t, child, 0xa6)) - GP4_GLOBAL(uint32_t, 0x0062ac50);
            flags = cmpFlags32(diff, 0x20u);
            const uint32_t value = static_cast<int32_t>(diff) < 0x20 ? GP4_GLOBAL(uint32_t, 0x0062fbfc) : 9u;
            r->eax = value;
            GP4_GLOBAL(uint32_t, 0x0062fbf8) = value;
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040f898, FUN_0040f898, "regs(eax:u32, edi:u32) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=layout=state_0040f898")
