#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears 0x6f3c00/0x6f3c01/0x6f3b88/0x6f3b8c, then (when bit 7 of 0x7ad88e is
// set and the random bit of 0x4015dc is clear) walks the car-order list
// 0x66fe3c forward from the selected car [0x679d30] (its list position +0x7d)
// to the 0xffff terminator. For each car on the same lap byte +0x70 that is
// behind by dx = +0x68 difference >= 0 (the walk stops once dx reaches the
// word 0x6f3bfe) and whose 0x408309 value is not more than 3 below the
// selected car's, the largest dx is kept; it is then scaled by 256, clamped to
// [0x6f3b80, 0x6f3b84] and stored in 0x6f3b8c with 0x6f3c01 = 0xff.
// pushal/popal keep every register; the flags are those of the last writer.

// FUNCTION: GP4 0x005254ac
void __cdecl FUN_005254ac(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x006f3c00) = 0;
    GP4_GLOBAL(uint8_t, 0x006f3c01) = 0;
    GP4_GLOBAL(uint32_t, 0x006f3b8c) = 0;
    GP4_GLOBAL(uint32_t, 0x006f3b88) = 0;
    const uint8_t enable = GP4_GLOBAL(uint8_t, 0x007ad88e);
    uint32_t flags = gp4::flags::logic8(enable & 0x80u);
    if ((enable & 0x80u) == 0) {
        r->eflags = gp4::flags::merge(r->eflags, flags);
        return;
    }
    gp4::Regs rng = {};
    rng.eax = r->eax;
    rng.eflags = r->eflags;
    gp4::call_regs(0x004015dc, rng);
    const uint32_t bit = rng.eax & 1u;
    flags = gp4::flags::logic32(bit);
    if (bit != 0) {
        r->eflags = gp4::flags::merge(r->eflags, flags);
        return;
    }
    const uint8_t* self = (const uint8_t*)GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint8_t* list = GP4_ARRAY(const uint8_t, 0x0066fe3c);
    uint32_t pos = self[0x7d];
    int32_t best = -1;
    for (;;) {
        pos += 2;
        const uint16_t off = *(const uint16_t*)(list + pos);
        if (off == 0xffffu) break;
        const uint8_t* other = GP4_ARRAY(const uint8_t, 0x0066fef0) + off;
        if (self[0x70] != other[0x70]) continue;
        const int16_t dx = (int16_t)(*(const int16_t*)(self + 0x68) - *(const int16_t*)(other + 0x68));
        if (dx < 0) continue;
        if (dx >= GP4_GLOBAL(int16_t, 0x006f3bfe)) break;
        gp4::Regs c = {};
        c.esi = (uint32_t)(uintptr_t)other;
        gp4::call_regs(0x00408309, c);
        const uint32_t otherPos = c.eax;
        c.esi = (uint32_t)(uintptr_t)self;
        gp4::call_regs(0x00408309, c);
        const int8_t delta = (int8_t)(uint8_t)(c.eax - otherPos);
        if (delta < -3) continue;
        if (best < (int32_t)dx) best = dx;
    }
    flags = gp4::flags::sub32((uint32_t)best, 0xffffffffu);
    if (best != -1) {
        int32_t v = best << 8;
        const int32_t lo = GP4_GLOBAL(int32_t, 0x006f3b80);
        if (v < lo) v = lo;
        const int32_t hi = GP4_GLOBAL(int32_t, 0x006f3b84);
        flags = gp4::flags::sub32((uint32_t)v, (uint32_t)hi);
        if (v > hi) v = hi;
        GP4_GLOBAL(int32_t, 0x006f3b8c) = v;
        GP4_GLOBAL(uint8_t, 0x006f3c01) = 0xff;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005254ac, FUN_005254ac, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0044334d+list_0066fe3c+lap_0043b482+scan_00409aec+state_0062d1b4")
