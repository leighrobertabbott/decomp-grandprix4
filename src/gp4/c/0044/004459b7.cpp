#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of the 8-bit CMP left, right.
static inline uint32_t cmpFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// When the pending byte 0x007a8df8 is 0xff: ANDs the mask byte 0x0062f8fd into the byte 0x0062f8fe and, if bit 6 of
// byte 0x0062f8ed is set, sets bit [byte 0x0062f8ea] of the bit string that starts at 0x0062f8e0 (`bts dword`).
// eax is saved and restored; the flags are those of the last flag-setting instruction (bts: CF = old bit, the other
// flags those of the `test`).
// FUNCTION: GP4 0x004459b7
void __cdecl FUN_004459b7(gp4::Regs* r) {
    const uint8_t pending = GP4_GLOBAL(uint8_t, 0x007a8df8);
    uint32_t flags = cmpFlags8(pending, 0xffu);
    if (pending == 0xffu) {
        const uint8_t anded = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0062f8fe) & GP4_GLOBAL(uint8_t, 0x0062f8fd));
        GP4_GLOBAL(uint8_t, 0x0062f8fe) = anded;
        const uint32_t gate = GP4_GLOBAL(uint8_t, 0x0062f8ed) & 0x40u;
        flags = logicFlags8(gate);
        if (gate != 0u) {
            const uint32_t bit = GP4_GLOBAL(uint8_t, 0x0062f8ea);
            uint32_t* word = GP4_ARRAY(uint32_t, 0x0062f8e0) + (bit >> 5);
            const uint32_t mask = 1u << (bit & 31u);
            flags = (flags & ~1u) | ((*word & mask) != 0u ? 1u : 0u);
            *word |= mask;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004459b7, FUN_004459b7, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bits_0062f8e0")
