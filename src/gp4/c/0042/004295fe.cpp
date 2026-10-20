#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t ParityFlag(uint32_t v) {
    uint32_t b = v & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t SubFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | ParityFlag(result) | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u) |
           (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u) |
           ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a 32-bit `add left, right`.
static inline uint32_t AddFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | ParityFlag(result) | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u) |
           (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u) |
           ((((left ^ result) & (right ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// One step forward along the 0x160-byte segment ring at 0x008943a0 (wrap at the lap end, clamp at the pit end).
static inline uint32_t NextSegment(uint32_t p) {
    p += 0x160u;
    if (p >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
        if (p < GP4_GLOBAL(uint32_t, 0x00891500)) {
            p -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        } else if (p >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            p = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    }
    return p;
}

static inline int16_t SegWord(uint32_t p, uint32_t off) {
    return *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(p + off));
}

// FUNCTION: GP4 0x004295fe
// pushal/popal routine for the car in esi. When the look-ahead segment [0x601b24] and the car's segment
// (+0x1c) are both before the pit start [0x891500], and the largest |word +0x14| of the car's next four
// segments reaches [0x601b7c], it derives a reach [0x601b80] = max([0x601b74] - |[0x601b70] * S >> 14|,
// [0x601b78]) from the summed segment word deltas S, then walks forward from [0x601b24] (fraction
// [0x601b20] first, then 0), mapping each segment point through 0x0040362e / 0x00404796 and measuring
// its distance to (0x601ad4, 0x601ad8) with fsqrt (>> 4), until the distance reaches the reach. When it
// stepped at least once, [0x601b24] becomes the last segment before it and [0x601b20] the 14-bit
// fraction where the reach is crossed (blended with the old fraction after a single step). Registers
// are restored by popal; the flags are those of the last comparison / addition.
void __cdecl FUN_004295fe(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);
    const uint32_t lookAhead = GP4_GLOBAL(uint32_t, 0x00601b24);
    if (lookAhead >= pitStart) {
        r->eflags = (r->eflags & ~0x8d5u) | SubFlags32(lookAhead, pitStart);
        return;
    }
    const uint32_t carSegment = GP4_FIELD(uint32_t, car, 0x1c);
    const uint32_t pitStart2 = GP4_GLOBAL(uint32_t, 0x00891500);
    if (carSegment >= pitStart2) {
        r->eflags = (r->eflags & ~0x8d5u) | SubFlags32(carSegment, pitStart2);
        return;
    }

    int32_t widest = 0;
    uint32_t seg = carSegment;
    for (int i = 0; i < 4; ++i) {
        int32_t w = SegWord(seg, 0x14);
        if (w < 0) w = -w;
        if (widest < w) widest = w;
        seg = NextSegment(seg);
    }
    const uint32_t minWidth = GP4_GLOBAL(uint32_t, 0x00601b7c);
    if (widest < (int32_t)minWidth) {
        r->eflags = (r->eflags & ~0x8d5u) | SubFlags32((uint32_t)widest, minWidth);
        return;
    }

    uint32_t bend = 0;
    seg = carSegment;
    for (int i = 0; i < 4; ++i) {
        const uint16_t d = (uint16_t)((uint16_t)SegWord(seg, 0x160) + (uint16_t)SegWord(seg, 0x178) -
                                      (uint16_t)SegWord(seg, 0) - (uint16_t)SegWord(seg, 0x18));
        bend += (uint32_t)(int32_t)(int16_t)d;
        seg = NextSegment(seg);
    }
    uint32_t cut = (uint32_t)((uint64_t)((int64_t)GP4_GLOBAL(int32_t, 0x00601b70) * (int64_t)(int32_t)bend) >> 14);
    if ((int32_t)cut < 0) cut = 0u - cut;
    uint32_t reach = GP4_GLOBAL(uint32_t, 0x00601b74) - cut;
    if ((int32_t)reach < GP4_GLOBAL(int32_t, 0x00601b78)) reach = GP4_GLOBAL(uint32_t, 0x00601b78);
    GP4_GLOBAL(uint32_t, 0x00601b80) = reach;
    GP4_GLOBAL(uint32_t, 0x00601b8c) = 0u;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.edi = GP4_GLOBAL(uint32_t, 0x00601b24);
    s.ebx = s.edi;
    uint32_t fraction = GP4_GLOBAL(uint32_t, 0x00601b20);
    int32_t distance;
    for (;;) {
        s.eax = fraction;
        gp4::call_regs(0x0040362e, s);
        s.ecx = s.eax;
        s.edx = fraction;
        gp4::call_regs(0x00404796, s);

        const int32_t dx = (int32_t)(s.eax - GP4_GLOBAL(uint32_t, 0x00601ad4));
        const int32_t dy = (int32_t)(s.edx - GP4_GLOBAL(uint32_t, 0x00601ad8));
        GP4_GLOBAL(int32_t, 0x006f2bf4) = dx;
        gp4::x87::Ext xx = gp4::x87::ext_i32(dx);
        xx = xx * xx;
        GP4_GLOBAL(int32_t, 0x006f2bf4) = dy;
        gp4::x87::Ext yy = gp4::x87::ext_i32(dy);
        yy = yy * yy;
        const int32_t length = gp4::x87::fistp32(gp4::x87::fsqrt(xx + yy));
        GP4_GLOBAL(int32_t, 0x006f2bf4) = length;
        distance = length >> 4;
        if (distance >= GP4_GLOBAL(int32_t, 0x00601b80)) break;

        GP4_GLOBAL(int32_t, 0x00601b88) = distance;
        s.ebx = s.edi;
        s.edi = NextSegment(s.edi);
        GP4_GLOBAL(uint32_t, 0x00601b8c) += 1u;
        fraction = 0u;
    }
    GP4_GLOBAL(int32_t, 0x00601b84) = distance;

    const uint32_t steps = GP4_GLOBAL(uint32_t, 0x00601b8c);
    if (steps == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | SubFlags32(steps, 0u);
        return;
    }

    int32_t span = (int32_t)((uint32_t)GP4_GLOBAL(int32_t, 0x00601b84) - (uint32_t)GP4_GLOBAL(int32_t, 0x00601b88));
    if (span < 1) span = 1;
    int32_t part = (int32_t)(GP4_GLOBAL(uint32_t, 0x00601b80) - (uint32_t)GP4_GLOBAL(int32_t, 0x00601b88));
    if (part < 0) part = 0;
    if (part > span) part = span;
    int32_t rem = 0;
    const int32_t blend = gp4::idiv64((int64_t)part * 0x4000, span, &rem);

    const uint32_t stepsNow = GP4_GLOBAL(uint32_t, 0x00601b8c);
    if (stepsNow == 1u) {
        const uint32_t old = GP4_GLOBAL(uint32_t, 0x00601b20);
        const uint32_t scaled =
            (uint32_t)((uint64_t)((int64_t)(int32_t)(0x4000u - old) * (int64_t)blend) >> 14);
        const uint32_t oldAgain = GP4_GLOBAL(uint32_t, 0x00601b20);
        GP4_GLOBAL(uint32_t, 0x00601b20) = scaled + oldAgain;
        GP4_GLOBAL(uint32_t, 0x00601b24) = s.ebx;
        flags = AddFlags32(scaled, oldAgain);
    } else {
        GP4_GLOBAL(uint32_t, 0x00601b20) = (uint32_t)blend;
        GP4_GLOBAL(uint32_t, 0x00601b24) = s.ebx;
        flags = SubFlags32(stepsNow, 1u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004295fe, FUN_004295fe, "regs(esi:ptr[0x298]:layout=car_00431ffb+car_0040362e+car_00426fa5+rec_004295fe) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+state_004295fe")
