#include <gp4/gp4.h>
#include <gp4/flags.h>

// Register routine (called between pushal/popal by 0x005427f9). Stores [0x00831333] through
// 0x00531af4, gets a step in ebx from 0x00542c43, then walks two grids:
//  - ticks 0, step, 2*step, ... while tick <= [0x008310c4] (unsigned): ticks below
//    [0x008310c0] (signed) are skipped, the walk stops once tick - [0x008310c0] > [0x008310c8];
//    otherwise y = ((tick - c0) * 0x278 * [0x0083137b] / [0x008310c4] >> 14) + 0x64 (0 quotient
//    when the divisor is zero) is passed to 0x00531b29 with eax = 0x60, ecx = 0x19b
//    (ebx/ecx are restored around the call);
//  - t = 0, 0xcf647, ... while t <= 0x67b23a: x = ((t - [0x008310cc]) * 0x13c * [0x0083137f] /
//    [0x008310d0] >> 14) + 0x60; when 0x60 <= x <= 0x19c, 0x00531b27 gets eax = x, ebx = 0x64,
//    ecx = 0x2db.
// Outputs: eax = final t, edx = last imul/idiv result, ebx/ecx as left by the walks, flags of
// the final `cmp eax, 0x67b23a`.
// FUNCTION: GP4 0x00542aa8
void __cdecl FUN_00542aa8(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = GP4_GLOBAL(uint32_t, 0x00831333);
    gp4::call_regs(0x00531af4, c);
    gp4::call_regs(0x00542c43, c);

    const uint32_t step = c.ebx;
    uint32_t tick = 0;
    for (;;) {
        c.ecx = tick;
        const uint32_t offset = tick - GP4_GLOBAL(uint32_t, 0x008310c0);
        c.eax = offset;
        if ((int32_t)tick >= GP4_GLOBAL(int32_t, 0x008310c0)) {
            if ((int32_t)offset > GP4_GLOBAL(int32_t, 0x008310c8)) {
                break;
            }
            const int32_t scaled = (int32_t)(offset * 0x278u);
            const int64_t product = (int64_t)scaled * (int64_t)GP4_GLOBAL(int32_t, 0x0083137b);
            const int32_t range = GP4_GLOBAL(int32_t, 0x008310c4);
            int32_t pos;
            if (range != 0) {
                int32_t rem;
                pos = gp4::idiv64(product, range, &rem);
                c.edx = (uint32_t)rem;
            } else {
                pos = 0;
                c.edx = (uint32_t)(product >> 32);
            }
            c.ebx = (uint32_t)((pos >> 14) + 0x64);
            c.eax = 0x60u;
            c.ecx = 0x19bu;
            gp4::call_regs(0x00531b29, c);
        }
        c.ebx = step;
        tick += step;
        c.ecx = tick;
        if (tick > GP4_GLOBAL(uint32_t, 0x008310c4)) {
            break;
        }
    }

    uint32_t t = 0;
    do {
        const int32_t scaled = (int32_t)((t - GP4_GLOBAL(uint32_t, 0x008310cc)) * 0x13cu);
        const int64_t product = (int64_t)scaled * (int64_t)GP4_GLOBAL(int32_t, 0x0083137f);
        const int32_t range = GP4_GLOBAL(int32_t, 0x008310d0);
        int32_t pos;
        if (range != 0) {
            int32_t rem;
            pos = gp4::idiv64(product, range, &rem);
            c.edx = (uint32_t)rem;
        } else {
            pos = 0;
            c.edx = (uint32_t)(product >> 32);
        }
        const int32_t x = (pos >> 14) + 0x60;
        if (x >= 0x60 && x <= 0x19c) {
            c.eax = (uint32_t)x;
            c.ebx = 0x64u;
            c.ecx = 0x2dbu;
            gp4::call_regs(0x00531b27, c);
        }
        t += 0xcf647u;
    } while (t <= 0x67b23au);

    c.eax = t;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(t, 0x67b23au));
    *r = c;
}
GP4_IMPL(0x00542aa8, FUN_00542aa8, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bounds_00542c43+grid_00542aa8")
