#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit add.
static inline uint32_t additionFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left + right);
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of a 16-bit "test value, 0xffff" (CF = OF = AF = 0).
static inline uint32_t testFlags16(uint16_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

// The numerator is sign-extended (cwd), shifted left 11 into dx:ax and divided by the divisor unless the
// high word is already >= the divisor (then the quotient register keeps the low word). The result is the
// byte offset (quotient * 2) into the word table at 0x007c7c60.
static inline uint16_t ratioOffset(uint16_t numerator, uint16_t divisor) {
    const uint32_t sign = (static_cast<int16_t>(numerator) < 0) ? 0xffu : 0u;
    const uint32_t dividend = ((sign << 24) | (static_cast<uint32_t>(numerator) << 8)) << 3;
    const uint16_t high = static_cast<uint16_t>(dividend >> 16);
    uint16_t quotient = static_cast<uint16_t>(dividend);
    if (!(high >= divisor)) {
        quotient = static_cast<uint16_t>(dividend / divisor);
    }
    return static_cast<uint16_t>(quotient << 1);
}

static inline uint16_t tableWord(uint16_t offset) {
    return *reinterpret_cast<const uint16_t*>(GP4_ARRAY(const uint8_t, 0x007c7c60) + offset);
}

// FUNCTION: GP4 0x00409f2a
// Angle of the vector (dx, ax) as a 16-bit Q-angle (a full turn = 0x10000, 0x4000 = a quarter): atan of the
// smaller magnitude over the larger one through the word table at 0x007c7c60 (index = ratio * 2048, so
// 0x4000 minus the table word when |ax| >= |dx|), negated when the signs differ and shifted by 0x8000 when
// dx is negative. A zero larger magnitude (the zero vector) returns 0, with the flags of the "test"
// that found it. Only ax changes (the rest is saved/restored); the flags are those of the final
// "test cx" or "add ax, 0x8000".
void __cdecl FUN_00409f2a(gp4::Regs* r) {
    const uint16_t y = static_cast<uint16_t>(r->eax);
    const uint16_t x = static_cast<uint16_t>(r->edx);

    uint16_t absY = y;
    if (static_cast<int16_t>(absY) < 0) {
        absY = static_cast<uint16_t>(0u - absY);
    }
    uint16_t absX = x;
    if (static_cast<int16_t>(absX) < 0) {
        absX = static_cast<uint16_t>(0u - absX);
    }
    const uint16_t signs = static_cast<uint16_t>(y ^ x);

    uint16_t angle;
    if (!(static_cast<int16_t>(absY) < static_cast<int16_t>(absX))) {
        if (absY == 0u) {
            r->eax = r->eax & 0xffff0000u;
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
            return;
        }
        angle = static_cast<uint16_t>(0x4000u - tableWord(ratioOffset(absX, absY)));
    } else {
        if (absX == 0u) {
            r->eax = r->eax & 0xffff0000u;
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
            return;
        }
        angle = tableWord(ratioOffset(absY, absX));
    }

    if (static_cast<int16_t>(signs) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }

    uint32_t flags;
    if (static_cast<int16_t>(x) < 0) {
        flags = additionFlags16(angle, 0x8000u);
        angle = static_cast<uint16_t>(angle + 0x8000u);
    } else {
        flags = testFlags16(x);
    }

    r->eax = (r->eax & 0xffff0000u) | angle;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00409f2a, FUN_00409f2a, "regs(eax:u32[0..0xffff8000], edx:u32[0..0xffff8000]) -> (eax, cf, pf, af, zf, sf, of); cases=600")
