#include <gp4/gp4.h>
#include <gp4/flags.h>

// Converts the 16-bit RGB triple at 0x00933a14/16/18 to HSV words: value 0x933a1e = max channel,
// saturation 0x933a1c = (max - min) * 0x4000 / max, hue 0x933a1a = the sector difference
// ((g - b), (b - r) + 2d or (r - g) + 4d by the maximal channel, d = max - min) scaled by
// 0x8000 / 3d, i.e. 0x10000 per turn, sign kept. pushal/popal keep every register; the flags are
// those of the last writer: `cmp esi, 0` (black), `test word [0x933a1c]` (grey), `or ebp, ebp` or
// the final `neg eax` for a negative hue difference.
// FUNCTION: GP4 0x0051b1f9
void __cdecl FUN_0051b1f9(gp4::Regs* r) {
    const uint32_t red = GP4_GLOBAL(uint16_t, 0x00933a14);
    const uint32_t green = GP4_GLOBAL(uint16_t, 0x00933a16);
    const uint32_t blue = GP4_GLOBAL(uint16_t, 0x00933a18);

    uint32_t high = red;
    if (high < green) {
        high = green;
    }
    if (high < blue) {
        high = blue;
    }
    uint32_t low = red;
    if (low >= green) {
        low = green;
    }
    if (low >= blue) {
        low = blue;
    }

    GP4_GLOBAL(uint16_t, 0x00933a1e) = static_cast<uint16_t>(high);
    if (high == 0u) {
        GP4_GLOBAL(uint16_t, 0x00933a1c) = 0;
        GP4_GLOBAL(uint16_t, 0x00933a1a) = 0;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(high, 0u));
        return;
    }

    const uint32_t delta = high - low;
    GP4_GLOBAL(uint16_t, 0x00933a1c) = static_cast<uint16_t>((delta << 14) / high);
    const uint32_t saturation = GP4_GLOBAL(uint16_t, 0x00933a1c);
    if (saturation == 0u) {
        GP4_GLOBAL(uint16_t, 0x00933a1a) = 0;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic16(saturation));
        return;
    }

    int32_t difference;
    if (static_cast<uint16_t>(high) == GP4_GLOBAL(uint16_t, 0x00933a14)) {
        difference = static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00933a16) - GP4_GLOBAL(uint16_t, 0x00933a18));
    } else if (static_cast<uint16_t>(high) == GP4_GLOBAL(uint16_t, 0x00933a16)) {
        difference = static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00933a18) - GP4_GLOBAL(uint16_t, 0x00933a14));
        difference = static_cast<int32_t>(static_cast<uint32_t>(difference) + (delta << 1));
    } else {
        difference = static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00933a14) - GP4_GLOBAL(uint16_t, 0x00933a16));
        difference = static_cast<int32_t>(static_cast<uint32_t>(difference) + (delta << 1) + (delta << 1));
    }

    uint32_t magnitude = static_cast<uint32_t>(difference);
    if (difference < 0) {
        magnitude = 0u - magnitude;
    }
    uint32_t hue = (magnitude << 15) / (delta * 3u);
    uint32_t arith = gp4::flags::logic32(static_cast<uint32_t>(difference));
    if (difference < 0) {
        arith = gp4::flags::neg32(hue);
        hue = 0u - hue;
    }
    GP4_GLOBAL(uint16_t, 0x00933a1a) = static_cast<uint16_t>(hue);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051b1f9, FUN_0051b1f9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rgb_0051b1f9")
