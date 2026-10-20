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

// FUNCTION: GP4 0x00403ec6
// Rotates the point (x, y) = words [esi + 0x90], [esi + 0x92] by the angle word [edi] (cosine and sine
// from the word table 0x007c5c5c, kept in 0x00934de8 and 0x00934dec = -sin) as 32-bit sums
//   X = x * cos - y * dec,  Y = y * cos + x * dec   (high words kept in 0x00934df2 / 0x00934df6),
// then takes (X >> 14) and (Y >> 14) as words (kept in 0x00934df0 / 0x00934df4), shifts each right by 3,
// adds the words [edi + 4] / [edi + 8] and splits the 16-bit sum d into a 16.5 fixed-point pair
// (low word (d & 31) << 11, high word d >> 5) stored to [esi], [esi + 2] and [esi + 4], [esi + 6].
// The last "rcr ax, 1" and the final stores of the y pair are the fall-through code of 0x00404040.
void __cdecl FUN_00403ec6(gp4::Regs* r) {
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* in = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const int32_t x = static_cast<int16_t>(GP4_FIELD(uint16_t, out, 0x90));
    const int32_t y = static_cast<int16_t>(GP4_FIELD(uint16_t, out, 0x92));

    const uint16_t angle = GP4_FIELD(uint16_t, in, 0);
    const int16_t cosine = tableWord(angle);
    const int16_t negSine = static_cast<int16_t>(0 - tableWord(static_cast<uint16_t>(0x4000u - angle)));
    GP4_GLOBAL(int16_t, 0x00934de8) = cosine;
    GP4_GLOBAL(int16_t, 0x00934dec) = negSine;

    const uint32_t sumX = static_cast<uint32_t>(x * cosine) - static_cast<uint32_t>(y * negSine);
    const uint32_t sumY = static_cast<uint32_t>(y * cosine) + static_cast<uint32_t>(x * negSine);

    GP4_GLOBAL(uint16_t, 0x00934df2) = static_cast<uint16_t>(sumX >> 16);
    GP4_GLOBAL(uint16_t, 0x00934df6) = static_cast<uint16_t>(sumY >> 16);
    const uint16_t wordX = static_cast<uint16_t>(sumX >> 14);
    const uint16_t wordY = static_cast<uint16_t>(sumY >> 14);
    GP4_GLOBAL(uint16_t, 0x00934df0) = wordX;
    GP4_GLOBAL(uint16_t, 0x00934df4) = wordY;

    // x: d = (wordX >> 3, arithmetic) + [edi + 4]; ax = (d & 31) << 11, dx = d >> 5
    const uint16_t dX = static_cast<uint16_t>((static_cast<int16_t>(wordX) >> 3) + GP4_FIELD(int16_t, in, 4));
    GP4_FIELD(uint16_t, out, 0) = static_cast<uint16_t>((dX & 31u) << 11);
    GP4_FIELD(uint16_t, out, 2) = static_cast<uint16_t>(static_cast<int16_t>(dX) >> 5);

    // y: the same, up to the fifth "sar dx, 1" - the fifth "rcr ax, 1" is in 0x00404040
    const uint16_t dY = static_cast<uint16_t>((static_cast<int16_t>(wordY) >> 3) + GP4_FIELD(int16_t, in, 8));
    const uint16_t highY = static_cast<uint16_t>(static_cast<int16_t>(dY) >> 5);
    const uint16_t lowY = static_cast<uint16_t>((dY & 15u) << 12);
    const uint32_t flags = ((dY >> 4) & 1u)                   // CF = last bit shifted out; OF = AF = 0
        | parityFlag(highY)
        | (highY == 0u ? 0x40u : 0u)
        | ((highY & 0x8000u) != 0u ? 0x80u : 0u);

    gp4::Regs tail = *r;
    tail.st_in = 0;
    tail.st_out = 0;
    tail.eax = lowY;
    tail.edx = highY;
    tail.eflags = (r->eflags & ~0x8d5u) | flags;
    gp4::call_regs(0x00404040, tail);

    r->eax = tail.eax;
    r->edx = tail.edx;
    r->eflags = tail.eflags;
}
GP4_IMPL(0x00403ec6, FUN_00403ec6, "regs(edi:ptr[0x10]:bytes, esi:ptr[0x9a]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
