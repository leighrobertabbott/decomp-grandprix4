#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Word of the Q14 cosine table 0x007c5c5c for a 16-bit angle: |angle| (16-bit negate), then
// (|angle| >> 2) & 0xfffe is the byte offset into the table.
static inline int16_t tableWord(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const uint32_t offset = (static_cast<uint32_t>(angle >> 2)) & 0xfffeu;
    return *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(0x007c5c5cu + offset));
}

// FUNCTION: GP4 0x004041dd
// Inverse-direction twin of 0x00403ec6 / 0x00403d57: the 16.5 fixed-point pairs at [esi] / [esi + 2]
// and [esi + 4] / [esi + 6] give the words (dx:ax << 5) >> 16; subtracting the words [edi + 4] / [edi + 8]
// and scaling by 8 gives x and y (kept in 0x00934df0 / 0x00934df4). With the cosine C and sine S of the
// angle word [edi] (table 0x007c5c5c, kept in 0x00934de8 / 0x00934dec):
//   X = x * C - y * S,  Y = y * C + x * S   (32-bit, high words kept in 0x00934df2 / 0x00934df6)
// and (X >> 14), (Y >> 14) as words go to [esi + 0x90], [esi + 0x92] (and 0x00934df0 / 0x00934df4).
// Leaves ax = low word of X << 2, dx = (X >> 14) as a word (high words of eax / edx kept), flags of
// the last "rcl dx, 1"; ebx is saved and restored.
void __cdecl FUN_004041dd(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* angleBlock = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const uint16_t xLow = GP4_FIELD(uint16_t, record, 0);
    const uint16_t xHigh = GP4_FIELD(uint16_t, record, 2);
    const uint16_t xWord = static_cast<uint16_t>(
        (static_cast<uint16_t>((xHigh << 5) | (xLow >> 11)) - GP4_FIELD(uint16_t, angleBlock, 4)) << 3);

    const uint16_t yLow = GP4_FIELD(uint16_t, record, 4);
    const uint16_t yHigh = GP4_FIELD(uint16_t, record, 6);
    const uint16_t yWord = static_cast<uint16_t>(
        (static_cast<uint16_t>((yHigh << 5) | (yLow >> 11)) - GP4_FIELD(uint16_t, angleBlock, 8)) << 3);

    const uint16_t angle = GP4_FIELD(uint16_t, angleBlock, 0);
    const int16_t cosine = tableWord(angle);
    const int16_t sine = tableWord(static_cast<uint16_t>(0x4000u - angle));
    GP4_GLOBAL(int16_t, 0x00934de8) = cosine;
    GP4_GLOBAL(int16_t, 0x00934dec) = sine;

    const int32_t x = static_cast<int16_t>(xWord);
    const int32_t y = static_cast<int16_t>(yWord);
    const uint32_t sumX = static_cast<uint32_t>(x * cosine) - static_cast<uint32_t>(y * sine);
    const uint32_t sumY = static_cast<uint32_t>(y * cosine) + static_cast<uint32_t>(x * sine);

    GP4_GLOBAL(uint16_t, 0x00934df2) = static_cast<uint16_t>(sumX >> 16);
    GP4_GLOBAL(uint16_t, 0x00934df6) = static_cast<uint16_t>(sumY >> 16);
    const uint16_t newX = static_cast<uint16_t>(sumX >> 14);
    const uint16_t newY = static_cast<uint16_t>(sumY >> 14);
    GP4_GLOBAL(uint16_t, 0x00934df0) = newX;
    GP4_GLOBAL(uint16_t, 0x00934df4) = newY;
    GP4_FIELD(uint16_t, record, 0x90) = newX;
    GP4_FIELD(uint16_t, record, 0x92) = newY;

    // Flags of the last "shl ax, 1 ; rcl dx, 1" pair on X (dx:ax = sumX << 2).
    const uint16_t lowX = static_cast<uint16_t>(sumX);
    const uint16_t highX = static_cast<uint16_t>(sumX >> 16);
    const uint16_t shifted = static_cast<uint16_t>(lowX << 2);
    const uint32_t carryOut = (highX >> 14) & 1u;
    const uint16_t rotated = newX;
    const uint32_t flags = parityFlag(shifted)
        | (shifted == 0u ? 0x40u : 0u)
        | ((shifted & 0x8000u) != 0u ? 0x80u : 0u)
        | carryOut
        | ((((rotated >> 15) & 1u) ^ carryOut) != 0u ? 0x800u : 0u);

    r->eax = (r->eax & 0xffff0000u) | shifted;
    r->edx = (r->edx & 0xffff0000u) | rotated;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004041dd, FUN_004041dd, "regs(eax:u32, edx:u32, edi:ptr[0x10]:bytes, esi:ptr[0x9a]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
