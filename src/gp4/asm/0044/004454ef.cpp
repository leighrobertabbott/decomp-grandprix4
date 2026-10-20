#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
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

// Flags of the 16-bit SUB left, right.
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x8000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// al/ah are two successive samples. A change arms a countdown (0x1c ticks); while they stay equal the
// countdown runs down and disarms the flag at 0x007a93b7 once it goes negative.
// FUNCTION: GP4 0x004454ef
void __cdecl FUN_004454ef(gp4::Regs* r) {
    const uint8_t low = static_cast<uint8_t>(r->eax);
    const uint8_t high = static_cast<uint8_t>(r->eax >> 8);
    uint32_t flags;

    GP4_GLOBAL(uint8_t, 0x0062b198) = low;
    GP4_GLOBAL(uint8_t, 0x0062b199) = high;
    flags = cmpFlags8(low, high);
    if (low != high) {
        GP4_GLOBAL(uint8_t, 0x007a93b7) = 0xff;
        GP4_GLOBAL(uint16_t, 0x007a93ba) = 0x1c;
    } else {
        const uint8_t armed = GP4_GLOBAL(uint8_t, 0x007a93b7);
        flags = cmpFlags8(armed, 0);
        if (armed != 0) {
            const uint16_t before = GP4_GLOBAL(uint16_t, 0x007a93ba);
            const uint16_t after = static_cast<uint16_t>(before - 1u);
            GP4_GLOBAL(uint16_t, 0x007a93ba) = after;
            flags = subFlags16(before, 1u);
            if ((after & 0x8000u) != 0u) {
                GP4_GLOBAL(uint8_t, 0x007a93b7) = 0;
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004454ef, FUN_004454ef, "regs(eax:u32[0..15]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
