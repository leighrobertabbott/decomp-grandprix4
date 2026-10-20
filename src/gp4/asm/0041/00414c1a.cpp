#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "add left, right"
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul + shrd eax, edx, shift: the low dword of the 64-bit product shifted right.
static inline uint32_t mulShift(int32_t a, int32_t b, int shift) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> shift);
}

// Q14 sine/cosine word table lookup: byte offset (angle >> 2) & 0xfffe.
static inline int32_t tableWord(uint32_t angle16) {
    const uint32_t offset = (angle16 >> 2) & 0xfffeu;
    return GP4_ARRAY(int16_t, 0x007c5c5c)[offset >> 1];
}

// FUNCTION: GP4 0x00414c1a
// pushal/popal wrapper: every register is restored, only the flags (those of the last
// "add [esi+4], eax") and two dwords of the record escape. With
// scale = ([0x623de0] * [0x60384c]) >> 8 and the heading
// angle = [edi+0x12e] + 0x4000 (or [edi+0x12c] - 0x4000 when the selector is negative),
// [esi] += cosTable(0x4000 - angle) * scale >> 14 and [esi+4] += sinTable(angle) * scale >> 14.
// The selector is the sword [esi+0x90], or +1 / -1 (sign of the byte [0x868582]) when
// edi >= [0x891500].
void __cdecl FUN_00414c1a(gp4::Regs* r) {
    const uint32_t edi = r->edi;
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edi));

    const int32_t scale = static_cast<int32_t>(mulShift(GP4_GLOBAL(int32_t, 0x00623de0), GP4_GLOBAL(int32_t, 0x0060384c), 8));

    int32_t selector = GP4_FIELD(int16_t, rec, 0x90);
    if (edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        selector = 1;
        if ((GP4_GLOBAL(uint8_t, 0x00868582) & 0x80u) != 0u) selector = -1;
    }

    int32_t heading;
    if (selector < 0) {
        heading = static_cast<int32_t>(GP4_FIELD(int16_t, other, 0x12c)) - 0x4000;
    } else {
        heading = static_cast<int32_t>(GP4_FIELD(int16_t, other, 0x12e)) + 0x4000;
    }

    // cx = heading; ax = 0x4000 - heading, made positive as a 16-bit value
    uint32_t angle = static_cast<uint16_t>(heading);
    uint32_t complement = static_cast<uint16_t>(0x4000u - angle);
    if ((complement & 0x8000u) != 0u) complement = static_cast<uint16_t>(0u - complement);
    const int32_t first = tableWord(complement);
    if ((angle & 0x8000u) != 0u) angle = static_cast<uint16_t>(0u - angle);
    const int32_t second = tableWord(angle);

    const uint32_t deltaX = mulShift(first, scale, 0xe);
    const uint32_t oldX = GP4_FIELD(uint32_t, rec, 0);
    GP4_FIELD(uint32_t, rec, 0) = oldX + deltaX;

    const uint32_t deltaY = mulShift(second, scale, 0xe);
    const uint32_t oldY = GP4_FIELD(uint32_t, rec, 4);
    GP4_FIELD(uint32_t, rec, 4) = oldY + deltaY;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags(oldY, deltaY);
}
GP4_IMPL(0x00414c1a, FUN_00414c1a, "regs(edi:ptr[0x136]:bytes, esi:ptr[0x98]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
