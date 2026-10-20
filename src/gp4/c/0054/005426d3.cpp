#include <gp4/gp4.h>
#include <gp4/flags.h>

// Register routine (eax/ebx preserved by push/pop). Maps the four corners of the plot area
// (x 0x64..0x2dc, y 0x60..0x19c) back to data values:
//   value = base + ((pixel - origin) << 14) * range / (zoom * width)
// with x: zoom [0x0083137b], width 0x278, range [0x008310c4], base [0x008310c0];
//      y: zoom [0x0083137f], width 0x13c, range [0x008310d0], base [0x008310cc].
// When the 64-bit quotient would not fit (divisor <= high dword) the divisor itself is used and
// the remainder is 0. Results go to 0x0082fcd0..0x0082fcec (each twice), 0x0082fcf0 = 2.
// Outputs: edx = the last remainder, flags of the last `add eax, [0x008310cc]`.
// FUNCTION: GP4 0x005426d3

static inline uint32_t unmap(int32_t pixel, int32_t origin, uint32_t width, uint32_t zoom,
                             uint32_t range, uint32_t* base_out, uint32_t base, uint32_t* rem) {
    int32_t span = pixel - origin;
    if (span < 0) {
        span = 0;
    }
    const uint32_t divisor = zoom * width;
    const uint64_t n = (uint64_t)((uint32_t)span << 14) * (uint64_t)range;
    uint32_t q;
    if (divisor > (uint32_t)(n >> 32)) {
        q = gp4::div64(n, divisor, rem);
    } else {
        q = divisor;
        *rem = 0;
    }
    *base_out = q;
    return q + base;
}

void __cdecl FUN_005426d3(gp4::Regs* r) {
    uint32_t rem = 0;
    uint32_t q = 0;

    const uint32_t left = unmap(0x64, 0x64, 0x278, GP4_GLOBAL(uint32_t, 0x0083137b),
                                GP4_GLOBAL(uint32_t, 0x008310c4), &q,
                                GP4_GLOBAL(uint32_t, 0x008310c0), &rem);
    GP4_GLOBAL(uint32_t, 0x0082fcd0) = left;
    GP4_GLOBAL(uint32_t, 0x0082fce0) = left;

    const uint32_t top = unmap(0x60, 0x60, 0x13c, GP4_GLOBAL(uint32_t, 0x0083137f),
                               GP4_GLOBAL(uint32_t, 0x008310d0), &q,
                               GP4_GLOBAL(uint32_t, 0x008310cc), &rem);
    GP4_GLOBAL(uint32_t, 0x0082fcd4) = top;
    GP4_GLOBAL(uint32_t, 0x0082fce4) = top;

    const uint32_t right = unmap(0x2dc, 0x64, 0x278, GP4_GLOBAL(uint32_t, 0x0083137b),
                                 GP4_GLOBAL(uint32_t, 0x008310c4), &q,
                                 GP4_GLOBAL(uint32_t, 0x008310c0), &rem);
    GP4_GLOBAL(uint32_t, 0x0082fcd8) = right;
    GP4_GLOBAL(uint32_t, 0x0082fce8) = right;

    const uint32_t base_y = GP4_GLOBAL(uint32_t, 0x008310cc);
    const uint32_t bottom = unmap(0x19c, 0x60, 0x13c, GP4_GLOBAL(uint32_t, 0x0083137f),
                                  GP4_GLOBAL(uint32_t, 0x008310d0), &q, base_y, &rem);
    GP4_GLOBAL(uint32_t, 0x0082fcdc) = bottom;
    GP4_GLOBAL(uint32_t, 0x0082fcec) = bottom;

    GP4_GLOBAL(uint8_t, 0x0082fcf0) = 2;

    r->edx = rem;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(q, base_y));
}
GP4_IMPL(0x005426d3, FUN_005426d3, "regs() -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
