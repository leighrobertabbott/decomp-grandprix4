#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// One "imul ; shrd eax, edx, 14" step: low dword of (a * b) >> 14 on the signed 64-bit product.
static inline int32_t mulShr14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
}

// High dword (edx) of the signed 64-bit product left behind by "imul".
static inline int32_t mulHigh(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<int32_t>(static_cast<uint64_t>(product) >> 32);
}

// 0x0040119c on the low word of eax (the angle), returned as a sign-extended 16-bit value.
static inline int32_t trigWord(gp4::Regs* r, uint32_t angle) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = angle & 0xffffu;
    gp4::call_regs(0x0040119c, call);
    return static_cast<int16_t>(call.eax);
}

// FUNCTION: GP4 0x004044f2
// Rotates the 2-D point (words +0x90 and +0x92 of the record in esi, each << 8) by the angle word at
// [edi] (Q14: cos = f(angle), sin = f(0x4000 - angle), via 0x0040119c), then adds the offsets
// ((word +4 of edi << 3 | low 3 bits of byte +0x21) << 8, (word +8 << 3 | high nibble of +0x21) << 8)
// and stores the result at [esi] and [esi+4]. The working values stay in DAT_00934de8/ec/f0/f4.
// On exit eax = the new Y, edx = the high half of the last imul (x * -sin) and the flags are those of
// the final "add eax, ebp"; ecx and ebp are preserved.
void __cdecl FUN_004044f2(gp4::Regs* r) {
    uint8_t* target = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* rotation = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const int32_t x = static_cast<int32_t>(GP4_FIELD(int16_t, target, 0x90)) << 8;
    const int32_t y = static_cast<int32_t>(GP4_FIELD(int16_t, target, 0x92)) << 8;
    GP4_GLOBAL(int32_t, 0x00934df0) = x;
    GP4_GLOBAL(int32_t, 0x00934df4) = y;

    const uint32_t angle = GP4_FIELD(uint16_t, rotation, 0x00);
    const int32_t cosine = trigWord(r, angle);
    const int32_t sine = trigWord(r, (0x4000u - angle) & 0xffffu);
    const int32_t negSine = static_cast<int16_t>(0u - static_cast<uint32_t>(sine));
    GP4_GLOBAL(int32_t, 0x00934de8) = cosine;
    GP4_GLOBAL(int32_t, 0x00934dec) = negSine;

    const int32_t rotatedX = mulShr14(x, cosine) - mulShr14(y, negSine);
    const int32_t rotatedY = mulShr14(y, cosine) + mulShr14(x, negSine);
    const int32_t edx = mulHigh(x, negSine);

    const uint32_t offsetByte = GP4_FIELD(uint8_t, rotation, 0x21);
    const uint32_t newX = ((static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, rotation, 0x04))) << 3)
                           | (offsetByte & 7u)) << 8;
    const uint32_t newY = ((static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, rotation, 0x08))) << 3)
                           | (offsetByte >> 4)) << 8;

    const uint32_t finalX = newX + static_cast<uint32_t>(rotatedX);
    const uint32_t finalY = newY + static_cast<uint32_t>(rotatedY);
    GP4_GLOBAL(uint32_t, 0x00934df0) = finalX;
    GP4_GLOBAL(uint32_t, 0x00934df4) = finalY;

    GP4_FIELD(uint32_t, target, 0x00) = finalX;
    GP4_FIELD(uint32_t, target, 0x04) = finalY;

    r->eax = finalY;
    r->edx = static_cast<uint32_t>(edx);
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(newY, static_cast<uint32_t>(rotatedY));
}
GP4_IMPL(0x004044f2, FUN_004044f2, "regs(edi:ptr[0x40]:bytes, esi:ptr[0xa0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
