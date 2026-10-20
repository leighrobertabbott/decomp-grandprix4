#include <gp4/gp4.h>

struct Unknown_0040435d_Esi {
    uint16_t Unknown00;
    uint16_t Unknown02;
    uint16_t Unknown04;
    uint16_t Unknown06;
};

struct Unknown_0040435d_Edi {
    uint16_t Angle00;
    uint16_t Unknown02;
    uint16_t Unknown04;
    uint16_t Unknown06;
    uint16_t Unknown08;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Looks up the Q14 table word for |angle| as the original does: index = (|angle| >> 2) & 0xfffe.
static inline uint16_t tableWord(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const uint32_t index = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
    return *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(0x007c5c5cu + index));
}

// (hi:lo) as the word pair of a 32-bit product / sum.
static inline uint16_t lowWord(int32_t v) { return static_cast<uint16_t>(v); }
static inline uint16_t highWord(int32_t v) { return static_cast<uint16_t>(static_cast<uint32_t>(v) >> 16); }

// FUNCTION: GP4 0x0040435d
// Rotates the offset (esi point words) from the position in edi by the angle at edi+0: each point
// coordinate is a 32-bit value whose bits 11..26 (dx after shl 5) minus the word at edi+4 / edi+8 is
// scaled by 8 into the words at 0x00934df0 / 0x00934df4; with sine S = 0x00934de8 and cosine C =
// 0x00934dec (Q14 table words 0x007c5c5c) the pair (X*S - Y*C, Y*S + X*C) is built as 32-bit values in
// 0x00934df0/f2 and 0x00934df4/f6 and shifted left by 2; the upper word of each is stored back at
// 0x00934df0 and 0x00934df4. eax:dx hold the last pair (the difference) in ax/dx with their
// high halves unchanged. Flags are those of the last `shl ax, 1; rcl dx, 1`.
void __cdecl FUN_0040435d(gp4::Regs* r) {
    const Unknown_0040435d_Esi* point = reinterpret_cast<const Unknown_0040435d_Esi*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0040435d_Edi* origin = reinterpret_cast<const Unknown_0040435d_Edi*>(static_cast<uintptr_t>(r->edi));

    // x: (point[1]:point[0]) << 5 high word, minus origin[2], times 8.
    const uint32_t xPair = ((static_cast<uint32_t>(point->Unknown02) << 16) | point->Unknown00) << 5;
    const uint16_t x = static_cast<uint16_t>((static_cast<uint16_t>(xPair >> 16) - origin->Unknown04) << 3);
    GP4_GLOBAL(uint16_t, 0x00934df0) = x;

    const uint32_t yPair = ((static_cast<uint32_t>(point->Unknown06) << 16) | point->Unknown04) << 5;
    const uint16_t y = static_cast<uint16_t>((static_cast<uint16_t>(yPair >> 16) - origin->Unknown08) << 3);
    GP4_GLOBAL(uint16_t, 0x00934df4) = y;

    const uint16_t angle = origin->Angle00;
    const uint16_t sine = tableWord(angle);
    GP4_GLOBAL(uint16_t, 0x00934de8) = sine;
    const uint16_t cosine = tableWord(static_cast<uint16_t>(0x4000u - angle));
    GP4_GLOBAL(uint16_t, 0x00934dec) = cosine;

    const int32_t xs = static_cast<int32_t>(static_cast<int16_t>(x)) * static_cast<int32_t>(static_cast<int16_t>(sine));
    const int32_t yc = static_cast<int32_t>(static_cast<int16_t>(y)) * static_cast<int32_t>(static_cast<int16_t>(cosine));
    const int32_t ys = static_cast<int32_t>(static_cast<int16_t>(y)) * static_cast<int32_t>(static_cast<int16_t>(sine));
    const int32_t xc = static_cast<int32_t>(static_cast<int16_t>(x)) * static_cast<int32_t>(static_cast<int16_t>(cosine));

    // sub word [df0], ax; sbb word [df2], dx
    const uint16_t diffLow = static_cast<uint16_t>(lowWord(xs) - lowWord(yc));
    const uint16_t diffHigh = static_cast<uint16_t>(highWord(xs) - highWord(yc) - (lowWord(xs) < lowWord(yc) ? 1u : 0u));
    GP4_GLOBAL(uint16_t, 0x00934df2) = diffHigh;

    // add word [df4], ax; adc word [df6], dx
    const uint32_t sumLowWide = static_cast<uint32_t>(lowWord(ys)) + lowWord(xc);
    const uint16_t sumLow = static_cast<uint16_t>(sumLowWide);
    const uint16_t sumHigh = static_cast<uint16_t>(highWord(ys) + highWord(xc) + (sumLowWide >> 16));
    GP4_GLOBAL(uint16_t, 0x00934df6) = sumHigh;

    const uint32_t sumShifted = ((static_cast<uint32_t>(sumHigh) << 16) | sumLow) << 2;
    GP4_GLOBAL(uint16_t, 0x00934df4) = static_cast<uint16_t>(sumShifted >> 16);

    // Difference pair: two `shl ax, 1; rcl dx, 1` steps; the second pair decides the flags.
    const uint32_t diffPair = (static_cast<uint32_t>(diffHigh) << 16) | diffLow;
    const uint32_t firstShift = diffPair << 1;
    const uint16_t ax1 = static_cast<uint16_t>(firstShift);
    const uint16_t dx1 = static_cast<uint16_t>(firstShift >> 16);
    const uint32_t secondShift = diffPair << 2;
    const uint16_t ax2 = static_cast<uint16_t>(secondShift);
    const uint16_t dx2 = static_cast<uint16_t>(secondShift >> 16);
    GP4_GLOBAL(uint16_t, 0x00934df0) = dx2;

    // shl ax,1 sets ZF/SF/PF from ax2 (AF = 0); the final rcl dx,1 sets CF = dx1 bit 15 and
    // OF = bit 15 of the result xor CF.
    const uint32_t carry = (dx1 >> 15) & 1u;
    uint32_t flags = carry;
    flags |= parityFlag(ax2);
    flags |= ax2 == 0u ? 0x40u : 0u;
    flags |= (ax2 >> 8) & 0x80u;
    flags |= ((((dx2 >> 15) & 1u) ^ carry) != 0u) ? 0x800u : 0u;
    (void)ax1;

    r->eax = (r->eax & 0xffff0000u) | ax2;
    r->edx = (r->edx & 0xffff0000u) | dx2;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040435d, FUN_0040435d, "regs(eax:u32, edx:u32, esi:ptr[0x10]:bytes, edi:ptr[0x10]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
