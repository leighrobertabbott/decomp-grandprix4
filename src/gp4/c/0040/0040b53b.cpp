#include <gp4/gp4.h>

// One draw of the game's byte generator 0x00401462 (only the low byte of eax is used).
static inline uint32_t randomByte() {
    gp4::Regs draw = {};
    gp4::call_regs(0x00401462, draw);
    return draw.eax & 0xffu;
}

// (draw << 6) * factor >> 14, the low dword of the 64-bit product shifted by 14 (imul; shrd 14).
static inline int32_t scaleDraw(uint32_t draw, int32_t factor) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(draw << 6)) * factor;
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
}

// Picks a value in [0, limit) for the weather/pattern slot: the two limits come from 0x0051fbae
// (high in eax, low in edx). Unless pattern byte 0x7ad8b4 is set (then a percentage of the high
// limit), a random draw is scaled into the high range or, when the draw is below twice the word
// 0x62d248, into low + draw * (high - low). Both clamps are signed: below zero gives 0, above
// limit - 1 gives limit - 1.
// FUNCTION: GP4 0x0040b53b
uint32_t __cdecl FUN_0040b53b() {
    gp4::Regs limits = {};
    gp4::call_regs(0x0051fbae, limits);
    const int32_t high = static_cast<int32_t>(limits.eax);
    const int32_t low = static_cast<int32_t>(limits.edx);

    int32_t value = 0;
    bool plain = false;
    bool blend = false;
    if (GP4_GLOBAL(uint8_t, 0x007a8e0f) == 0 || GP4_GLOBAL(uint8_t, 0x007c2e75) != 0) {
        plain = true;
    } else if (GP4_GLOBAL(uint8_t, 0x007ad8b4) == 0xff) {
        const int64_t numerator = static_cast<int64_t>(GP4_GLOBAL(uint8_t, 0x007ad8b5)) * high;
        int32_t remainder;
        value = gp4::idiv64(numerator, 100, &remainder);
    } else {
        const int32_t scaled = scaleDraw(randomByte(), 100);
        const int32_t threshold = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x0062d248)) << 1);
        if (scaled < threshold) {
            blend = true;
        } else {
            plain = true;
        }
    }

    if (plain) {
        value = scaleDraw(randomByte(), low);
        if (value < 0) {
            value = 0;
        }
        const int32_t top = static_cast<int32_t>(static_cast<uint32_t>(low) - 1u);
        if (value > top) {
            value = top;
        }
        return static_cast<uint32_t>(value);
    }

    if (blend) {
        value = scaleDraw(randomByte(), static_cast<int32_t>(static_cast<uint32_t>(high) - static_cast<uint32_t>(low)));
        value = static_cast<int32_t>(static_cast<uint32_t>(value) + static_cast<uint32_t>(low));
    }
    if (value < 0) {
        value = 0;
    }
    const int32_t top = static_cast<int32_t>(static_cast<uint32_t>(high) - 1u);
    if (value > top) {
        value = top;
    }
    return static_cast<uint32_t>(value);
}
GP4_IMPL(0x0040b53b, FUN_0040b53b, "cdecl() -> u32; globals=fuzz:i32+layout=state_0066d8c0")
