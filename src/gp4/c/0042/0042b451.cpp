#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0042b451
void __cdecl FUN_0042b451(gp4::Regs* r) {
    const uint16_t word = GP4_GLOBAL(uint16_t, 0x00604404);
    const uint8_t sign = GP4_GLOBAL(uint8_t, 0x00868582);
    uint32_t flags;
    uint8_t result;
    if (((uint8_t)(word >> 8) ^ sign) & 0x80u) {
        // test word, 0xffff
        flags = parityFlag(word)
            | (word == 0u ? 0x40u : 0u)
            | ((word & 0x8000u) != 0u ? 0x80u : 0u);
        result = (word & 0x8000u) != 0u ? 0x80u : 1u;
    } else {
        uint16_t value = GP4_GLOBAL(uint16_t, 0x00603d9c);
        if ((sign & 0x80u) != 0u) {
            value = (uint16_t)(0u - value);
        }
        value = (uint16_t)(value + 0x300u);
        uint8_t d = sign;
        if (!((int16_t)value < (int16_t)GP4_GLOBAL(uint16_t, 0x00604408))) {
            d ^= 0x80u;
        }
        // or dl, dl
        flags = parityFlag(d)
            | (d == 0u ? 0x40u : 0u)
            | ((d & 0x80u) != 0u ? 0x80u : 0u);
        result = (d & 0x80u) != 0u ? 0x80u : 1u;
    }
    GP4_GLOBAL(uint8_t, 0x0060440c) = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042b451, FUN_0042b451, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
