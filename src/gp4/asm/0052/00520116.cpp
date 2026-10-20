#include <gp4/gp4.h>
#include <gp4/flags.h>

// Signed distance eax - edx between two ring positions (record addresses from 0x008943a0 up to the lap end
// [0x866ecc]), folded around the lap: [0x874e00] = 0 normally and 0xff when the half-lap threshold [0x866ed8]
// was reached and the lap length [0x866ec4] was subtracted. A position outside [0x8943a0, lap end) leaves eax
// unchanged and returns CF = 1. The pit position [0x866f10] chooses the direction: both positions on one side give
// eax - edx, otherwise the larger minus the smaller (edx = that difference, eax = its negation when edx is the larger
// one). The flags are those of the last flag writer with CF replaced by the clc/stc epilogue.
// FUNCTION: GP4 0x00520116
void __cdecl FUN_00520116(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00874e00) = 0;
    const uint32_t ringStart = 0x008943a0u;
    const uint32_t a = r->eax;
    const uint32_t d = r->edx;

    uint32_t flags = gp4::flags::sub32(a, ringStart);
    bool outside = a < ringStart;
    if (!outside) {
        flags = gp4::flags::sub32(d, ringStart);
        outside = d < ringStart;
        if (!outside) {
            const uint32_t lapEnd = GP4_GLOBAL(uint32_t, 0x00866ecc);
            flags = gp4::flags::sub32(a, lapEnd);
            outside = a >= lapEnd;
            if (!outside) {
                flags = gp4::flags::sub32(d, lapEnd);
                outside = d >= lapEnd;
            }
        }
    }
    if (outside) {
        r->eflags = gp4::flags::merge(r->eflags, (flags & ~1u) | 1u);
        return;
    }

    const uint32_t pit = GP4_GLOBAL(uint32_t, 0x00866f10);
    uint32_t outEax = a;
    uint32_t outEdx = d;
    flags = gp4::flags::sub32(a, pit);
    if (a >= pit) {
        flags = gp4::flags::sub32(d, pit);
        if (d >= pit) {
            outEax = a - d;
            flags = gp4::flags::sub32(a, d);
        } else {
            uint32_t span = a - d;
            const uint32_t half = GP4_GLOBAL(uint32_t, 0x00866ed8);
            flags = gp4::flags::sub32(span, half);
            if (static_cast<int32_t>(span) >= static_cast<int32_t>(half)) {
                GP4_GLOBAL(uint8_t, 0x00874e00) = 0xffu;
                const uint32_t wrap = GP4_GLOBAL(uint32_t, 0x00866ec4);
                flags = gp4::flags::sub32(span, wrap);
                span -= wrap;
            }
            outEax = span;
        }
    } else {
        flags = gp4::flags::sub32(d, pit);
        if (d < pit) {
            outEax = a - d;
            flags = gp4::flags::sub32(a, d);
        } else {
            uint32_t span = d - a;
            const uint32_t half = GP4_GLOBAL(uint32_t, 0x00866ed8);
            flags = gp4::flags::sub32(span, half);
            if (static_cast<int32_t>(span) >= static_cast<int32_t>(half)) {
                const uint32_t wrap = GP4_GLOBAL(uint32_t, 0x00866ec4);
                flags = gp4::flags::sub32(span, wrap);
                span -= wrap;
                GP4_GLOBAL(uint8_t, 0x00874e00) = 0xffu;
            }
            outEdx = span;
            outEax = 0u - span;
            flags = gp4::flags::neg32(span);
        }
    }
    r->eax = outEax;
    r->edx = outEdx;
    r->eflags = gp4::flags::merge(r->eflags, flags & ~1u);
}
GP4_IMPL(0x00520116, FUN_00520116, "regs(eax:u32{0x100,0x894240,0x8943a0,0x894500,0x894660,0x8947c0,0x894920,0x894a80,0x894be0,0x894d40,0x894ea0,0x895000}, edx:u32{0x100,0x894240,0x8943a0,0x894500,0x894660,0x8947c0,0x894920,0x894a80,0x894be0,0x894d40,0x894ea0,0x895000}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=lap_0040cf8c+f10_00520116+thr_00520116")
