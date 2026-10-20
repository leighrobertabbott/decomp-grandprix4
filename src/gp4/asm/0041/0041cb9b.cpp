#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags (without CF) of `dec dword` giving `result` from `previous`.
static inline uint32_t decFlags32(uint32_t previous, uint32_t result) {
    return parityFlag8(result)
        | (((previous ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (previous == 0x80000000u ? 0x800u : 0u);
}

// One step along the track-segment ring (0x160-byte records), forwards.
static inline uint32_t stepForward(uint32_t segment) {
    segment += 0x160u;
    if (segment < GP4_GLOBAL(uint32_t, 0x00866ecc)) {
        return segment;
    }
    if (segment < GP4_GLOBAL(uint32_t, 0x00891500)) {
        return segment - GP4_GLOBAL(uint32_t, 0x00866ec4);
    }
    if (segment < GP4_GLOBAL(uint32_t, 0x00866ef8)) {
        return segment;
    }
    return GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
}

// One step along the track-segment ring, backwards (clamped at the pit start, wrapped below the lap start).
static inline uint32_t stepBack(uint32_t segment) {
    if (segment < GP4_GLOBAL(uint32_t, 0x00891500)) {
        segment -= 0x160u;
        if (segment < GP4_GLOBAL(uint32_t, 0x00891504)) {
            segment += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
        return segment;
    }
    segment -= 0x160u;
    if (segment < GP4_GLOBAL(uint32_t, 0x00891500)) {
        segment = GP4_GLOBAL(uint32_t, 0x00891500);
    }
    return segment;
}

// Maps the stored point (0x623e24 / 0x623e2c) into the frame of the segment (0x0041c622); fills
// eax / ecx / edx / flags of `c` (CF set when the mapped y is not negative).
static inline void mapPoint(gp4::Regs* c, uint32_t segment) {
    c->eax = GP4_GLOBAL(uint32_t, 0x00623e24);
    c->edx = GP4_GLOBAL(uint32_t, 0x00623e2c);
    c->edi = segment;
    gp4::call_regs(0x0041c622, *c);
}

static inline void keepNegative(const gp4::Regs& c) {
    GP4_GLOBAL(uint32_t, 0x00602040) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00602044) = c.edx;
    GP4_GLOBAL(uint32_t, 0x0060204c) = c.ecx;
    GP4_GLOBAL(uint32_t, 0x00602050) = c.edi;
}

static inline void keepPositive(const gp4::Regs& c) {
    GP4_GLOBAL(uint32_t, 0x0060202c) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00602030) = c.edx;
    GP4_GLOBAL(uint32_t, 0x00602038) = c.ecx;
    GP4_GLOBAL(uint32_t, 0x0060203c) = c.edi;
}

// FUNCTION: GP4 0x0041cb9b
// Searches the track-segment ring around the segment in edi for the segment whose frame puts the
// point (eax, edx) on the other side of the line (mapped y of the other sign), at most 0x20 steps
// (counter [0x602060]). When the point maps to y < 0 it walks backwards until y >= 0 and leaves the
// two bracketing results in [0x602040..0x602050] (y < 0 side) and [0x60202c..0x60203c] (y >= 0
// side); when y >= 0 it walks forwards until y < 0 (sides swapped). Returns CF = 0 when the point
// was bracketed and CF = 1 (stc) when 0x20 steps found no sign change. eax / ecx / edx / edi are
// those of the last mapping; ebp / ebx / esi are restored; the other flags come from the last
// mapping (or the exhausted counter).
void __cdecl FUN_0041cb9b(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00623e24) = r->eax;
    GP4_GLOBAL(uint32_t, 0x00623e2c) = r->edx;
    GP4_GLOBAL(uint32_t, 0x00602060) = 0x20u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    mapPoint(&c, r->edi);

    bool exhausted = true;
    if ((c.eflags & 1u) == 0u) {
        // y < 0: walk backwards looking for y >= 0
        for (;;) {
            keepNegative(c);
            mapPoint(&c, stepBack(c.edi));
            if ((c.eflags & 1u) != 0u) {
                keepPositive(c);
                exhausted = false;
                break;
            }
            const uint32_t before = GP4_GLOBAL(uint32_t, 0x00602060);
            GP4_GLOBAL(uint32_t, 0x00602060) = before - 1u;
            if (before - 1u == 0u) {
                break;
            }
        }
    } else {
        // y >= 0: walk forwards looking for y < 0
        for (;;) {
            keepPositive(c);
            mapPoint(&c, stepForward(c.edi));
            if ((c.eflags & 1u) == 0u) {
                keepNegative(c);
                exhausted = false;
                break;
            }
            const uint32_t before = GP4_GLOBAL(uint32_t, 0x00602060);
            GP4_GLOBAL(uint32_t, 0x00602060) = before - 1u;
            if (before - 1u == 0u) {
                break;
            }
        }
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    uint32_t flags;
    if (exhausted) {
        // stc after the final `dec` (counter 1 -> 0)
        flags = decFlags32(1u, 0u) | 1u;
    } else {
        // clc after the last mapping's `or edx, edx`
        flags = c.eflags & 0x8d4u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041cb9b, FUN_0041cb9b, "regs(eax:i32[-0x100000..0x100000], edx:i32[-0x100000..0x100000], edi:rec[0x8943a0,0x160,0..14]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ranges_004322a6+ring_00405a12+links_00405a12+state_0040759e")
