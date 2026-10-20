#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit TEST (CF = OF = AF = 0) with result `value`.
static inline uint32_t testFlags8(uint8_t value) {
    return byteParity(value)
        | (value == 0u ? 0x040u : 0u)
        | ((value & 0x80u) != 0u ? 0x080u : 0u);
}

// Flags of the 32-bit logic op `or reg, reg`.
static inline uint32_t orFlags32(uint32_t value) {
    return byteParity(value)
        | (value == 0u ? 0x040u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x080u : 0u);
}

// Classifies the car [0x00852d30] into the word [0x007c2e94]: 0x8000 unless the mode byte
// [0x007c2ea9] is negative, [0x007aafd4] is zero and the car exists with bit 5 of byte +0x7a;
// otherwise 0, 1 (bit 5 of +0x81) or 2 (also bits 6 and 7 of +0x80). Registers are saved/restored
// (pushal/popal), so only the flags of the last test/add remain.
// FUNCTION: GP4 0x00442ed6
void __cdecl FUN_00442ed6(gp4::Regs* r) {
    uint16_t classification = 0x8000u;
    uint32_t flags;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    flags = testFlags8(mode);
    if ((mode & 0x80u) != 0u) {
        const uint8_t pitMode = GP4_GLOBAL(uint8_t, 0x007aafd4);
        flags = testFlags8(pitMode);
        if (pitMode == 0u) {
            const uint32_t carAddress = GP4_GLOBAL(uint32_t, 0x00852d30);
            flags = orFlags32(carAddress);
            if (carAddress != 0u) {
                const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(carAddress));
                const uint8_t bit5 = static_cast<uint8_t>(car[0x7a] & 0x20u);
                flags = testFlags8(bit5);
                if (bit5 != 0u) {
                    classification = 0u;
                    const uint8_t linked = static_cast<uint8_t>(car[0x81] & 0x20u);
                    flags = testFlags8(linked);
                    if (linked != 0u) {
                        classification = 1u;
                        flags = 0u;
                        const uint8_t low = static_cast<uint8_t>(car[0x80] & 0x40u);
                        flags = testFlags8(low);
                        if (low != 0u) {
                            const uint8_t high = static_cast<uint8_t>(car[0x80] & 0x80u);
                            flags = testFlags8(high);
                            if (high != 0u) {
                                classification = 2u;
                                flags = 0u;
                            }
                        }
                    }
                }
            }
        }
    }

    GP4_GLOBAL(uint16_t, 0x007c2e94) = classification;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00442ed6, FUN_00442ed6, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+carno_0066ff6c+cur_00852d30")
