#include <gp4/gp4.h>
#include <gp4/flags.h>

// (a * b) >> 14 of two 16-bit words, the full 32-bit product rebuilt from dx:ax; only the low word is kept.
static inline uint16_t Scale14(uint32_t a, uint32_t b) {
    return static_cast<uint16_t>(((a & 0xffffu) * (b & 0xffffu)) >> 14);
}

// Converts the hue/saturation/value words 0x00933a1a/1c/1e (the inverse of 0x0051b1f9, 0x4000 = 1.0)
// back to the 16-bit RGB triple 0x00933a14/16/18. A zero saturation gives grey (all three = value).
// Otherwise sector = (hue * 6) >> 16, f = the low word of hue * 6, and with s' = (f * s) >> 16:
// p = v * (0x4000 - s) >> 14, q = v * (0x4000 - s') >> 14, t = v * (0x4000 - s + s') >> 14 (16-bit
// differences); the sector picks (v,t,p) (q,v,p) (p,v,t) (p,q,v) (t,p,v) (v,p,q). pushal/popal keep
// every register; the flags are those of `or ebx, ebx` (grey) or of the last sector compare
// (`cmp bp, 1` below sector 3, `cmp bp, 4` from it).
// FUNCTION: GP4 0x0051b2ea
void __cdecl FUN_0051b2ea(gp4::Regs* r) {
    const uint32_t hue = GP4_GLOBAL(uint16_t, 0x00933a1a);
    const uint32_t saturation = GP4_GLOBAL(uint16_t, 0x00933a1c);
    const uint32_t value = GP4_GLOBAL(uint16_t, 0x00933a1e);

    if (saturation == 0u) {
        GP4_GLOBAL(uint16_t, 0x00933a14) = static_cast<uint16_t>(value);
        GP4_GLOBAL(uint16_t, 0x00933a16) = static_cast<uint16_t>(value);
        GP4_GLOBAL(uint16_t, 0x00933a18) = static_cast<uint16_t>(value);
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(saturation));
        return;
    }

    const uint32_t scaled = hue * 6u;
    const uint32_t sector = (scaled >> 16) & 0xffffu;
    const uint32_t fraction = scaled & 0xffffu;

    const uint16_t p = Scale14(0x4000u - GP4_GLOBAL(uint16_t, 0x00933a1c), GP4_GLOBAL(uint16_t, 0x00933a1e));
    const uint32_t part = (fraction * GP4_GLOBAL(uint16_t, 0x00933a1c)) >> 16;
    const uint16_t q = Scale14(0x4000u - part, GP4_GLOBAL(uint16_t, 0x00933a1e));
    const uint32_t part2 = (fraction * GP4_GLOBAL(uint16_t, 0x00933a1c)) >> 16;
    const uint16_t t = Scale14(part2 - GP4_GLOBAL(uint16_t, 0x00933a1c) + 0x4000u, GP4_GLOBAL(uint16_t, 0x00933a1e));
    const uint16_t v = GP4_GLOBAL(uint16_t, 0x00933a1e);

    uint16_t red, green, blue;
    uint32_t arith;
    if (sector < 3u) {
        arith = gp4::flags::sub16(sector, 1u);
        if (sector > 1u) {
            red = p; green = v; blue = t;
        } else if (sector == 1u) {
            red = q; green = v; blue = p;
        } else {
            red = v; green = t; blue = p;
        }
    } else {
        arith = gp4::flags::sub16(sector, 4u);
        if (sector > 4u) {
            red = v; green = p; blue = q;
        } else if (sector == 4u) {
            red = t; green = p; blue = v;
        } else {
            red = p; green = q; blue = v;
        }
    }
    GP4_GLOBAL(uint16_t, 0x00933a14) = red;
    GP4_GLOBAL(uint16_t, 0x00933a16) = green;
    GP4_GLOBAL(uint16_t, 0x00933a18) = blue;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051b2ea, FUN_0051b2ea, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
