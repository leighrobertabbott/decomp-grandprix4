#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Shifts the 32-bit pair dx:ax left by `count` bits through (shl ax,1 ; rcl dx,1) steps and returns the
// flags of the last step: PF/ZF/SF from the last `shl ax,1` (AF = 0), CF/OF from the last `rcl dx,1`.
static inline uint32_t shiftPairLeft(uint16_t* ax, uint16_t* dx, int count) {
    uint32_t flags = 0;
    for (int i = 0; i < count; ++i) {
        const uint32_t carry = (*ax >> 15) & 1u;
        const uint16_t shifted = static_cast<uint16_t>(*ax << 1);
        *ax = shifted;
        const uint32_t carryOut = (*dx >> 15) & 1u;
        const uint16_t rotated = static_cast<uint16_t>((*dx << 1) | carry);
        *dx = rotated;
        flags = parityFlag(shifted)
            | (shifted == 0u ? 0x40u : 0u)
            | ((shifted & 0x8000u) != 0u ? 0x80u : 0u)
            | carryOut
            | ((((rotated >> 15) & 1u) ^ carryOut) != 0u ? 0x800u : 0u);
    }
    return flags;
}

// Table index of a Q-angle: (|angle| >> 2) & 0xfffe, a byte offset into the cosine word table 0x007c5c5c.
static inline int16_t tableWord(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const uint32_t offset = (static_cast<uint32_t>(angle >> 2)) & 0xfffeu;
    return *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(0x007c5c5cu + offset));
}

// FUNCTION: GP4 0x00403d57
// Rotates the point held in the two dwords 0x00934df0 (x) and 0x00934df4 (y): each is taken as
// (dword >> 11) - the record's word (+4 / +8), times 8 (16-bit), then multiplied with the cosine and
// sine (word table 0x007c5c5c, kept in 0x00934de8 / 0x00934dec) of the angle word [edi] and its
// complement 0x4000 - angle. The 32-bit rotated sums are stored back shifted right by 14 into the low
// words (the high words keep the unshifted sums). Leaves ax = low word << 2, dx = the stored x word
// (high words of eax/edx are kept) and the flags of the last rcl.
void __cdecl FUN_00403d57(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint16_t& xLow = GP4_GLOBAL(uint16_t, 0x00934df0);
    uint16_t& xHigh = GP4_GLOBAL(uint16_t, 0x00934df2);
    uint16_t& yLow = GP4_GLOBAL(uint16_t, 0x00934df4);
    uint16_t& yHigh = GP4_GLOBAL(uint16_t, 0x00934df6);

    uint16_t x = static_cast<uint16_t>((xHigh << 5) | (xLow >> 11));
    x = static_cast<uint16_t>(x - GP4_FIELD(uint16_t, record, 4));
    xLow = static_cast<uint16_t>(x << 3);

    uint16_t y = static_cast<uint16_t>((yHigh << 5) | (yLow >> 11));
    y = static_cast<uint16_t>(y - GP4_FIELD(uint16_t, record, 8));
    yLow = static_cast<uint16_t>(y << 3);

    const uint16_t angle = GP4_FIELD(uint16_t, record, 0);
    const int16_t cosine = tableWord(angle);
    GP4_GLOBAL(int16_t, 0x00934de8) = cosine;
    const int16_t sine = tableWord(static_cast<uint16_t>(0x4000u - angle));
    GP4_GLOBAL(int16_t, 0x00934dec) = sine;

    const int32_t xs = static_cast<int16_t>(xLow);
    const int32_t ys = static_cast<int16_t>(yLow);

    // x' = x * cos - y * sin (32-bit in xHigh:xLow)
    const uint32_t xCos = static_cast<uint32_t>(xs * cosine);
    xLow = static_cast<uint16_t>(xCos);
    xHigh = static_cast<uint16_t>(xCos >> 16);
    const uint32_t ySin = static_cast<uint32_t>(ys * sine);
    {
        const uint16_t before = xLow;
        const uint16_t subLow = static_cast<uint16_t>(ySin);
        xLow = static_cast<uint16_t>(before - subLow);
        xHigh = static_cast<uint16_t>(xHigh - static_cast<uint16_t>(ySin >> 16) - (before < subLow ? 1u : 0u));
    }

    // y' = y * cos + x * sin (32-bit in yHigh:yLow)
    const uint32_t yCos = static_cast<uint32_t>(ys * cosine);
    yLow = static_cast<uint16_t>(yCos);
    yHigh = static_cast<uint16_t>(yCos >> 16);
    const uint32_t xSin = static_cast<uint32_t>(xs * sine);
    {
        const uint16_t before = yLow;
        const uint16_t addLow = static_cast<uint16_t>(xSin);
        yLow = static_cast<uint16_t>(before + addLow);
        yHigh = static_cast<uint16_t>(yHigh + static_cast<uint16_t>(xSin >> 16)
            + (static_cast<uint16_t>(before + addLow) < before ? 1u : 0u));
    }

    uint16_t ax = yLow;
    uint16_t dx = yHigh;
    shiftPairLeft(&ax, &dx, 2);
    yLow = dx;

    ax = xLow;
    dx = xHigh;
    const uint32_t flags = shiftPairLeft(&ax, &dx, 2);
    xLow = dx;

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->edx = (r->edx & 0xffff0000u) | dx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00403d57, FUN_00403d57, "regs(eax:u32, edx:u32, edi:ptr[0x40]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
