#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the one-operand `imul r/m32` (oracle model): CF = OF = the 64-bit product does not fit
// the low dword, PF/ZF/SF come from the low dword, AF = 0.
static inline uint32_t imulFlags32(int64_t product) {
    const uint32_t low = static_cast<uint32_t>(product);
    const bool overflow = product != static_cast<int64_t>(static_cast<int32_t>(low));
    return (overflow ? 0x801u : 0u)
        | parityFlag8(low)
        | (low == 0u ? 0x40u : 0u)
        | ((low & 0x80000000u) != 0u ? 0x80u : 0u);
}

// imul ; shrd eax, edx, 14: low dword of (a * b) >> 14.
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// Interpolation record edi: two parallel sets of end points (+0x100/+0x144/+0x146 ... for the
// edx < 0 side, +0x10c/+0x14a/+0x14c ... for the other), blended by a Q14 fraction.
struct Unknown_00406f05_Body {
    uint8_t Unknown00[0x100];
    int32_t Unknown100;
    uint8_t Unknown104[0x10c - 0x104];
    int32_t Unknown10C;
    uint8_t Unknown110[0x144 - 0x110];
    uint16_t Unknown144;
    int16_t Unknown146;
    uint8_t Unknown148[0x14a - 0x148];
    uint16_t Unknown14A;
    int16_t Unknown14C;
    uint8_t Unknown14E[0x260 - 0x14e];
    int32_t Unknown260;
    uint8_t Unknown264[0x26c - 0x264];
    int32_t Unknown26C;
    uint8_t Unknown270[0x2a4 - 0x270];
    uint16_t Unknown2A4;
    int16_t Unknown2A6;
    uint8_t Unknown2A8[0x2aa - 0x2a8];
    uint16_t Unknown2AA;
    int16_t Unknown2AC;
};

// FUNCTION: GP4 0x00406f05
// edx < 0 selects the +0x144 / +0x2a4 end points, otherwise +0x14a / +0x2aa. With the fraction
// t1 = round([0x627950] * [0x6f2b04]) (Q14) it stores P = [0x627820] = lerp(u16 a << 6, u16 b << 6)
// and Q = [0x627824] = lerp(s16 a << 6, s16 b << 6) and takes the dword lerp S of the third pair;
// with t2 = round([0x627958] * [0x6f2b04]) it evaluates u = (t2 * S) >> 14 and returns
// eax = u * Q / P when u < P, otherwise clamp(S - u, 0, B) * Q / B with B = max(min(S - P, S), 1)
// (a zero P counts as 1). edx = the remainder. ecx, ebp and esi are preserved; the flags are those
// of the imul before the idiv (idiv leaves them).
void __cdecl FUN_00406f05(gp4::Regs* r) {
    using namespace gp4::x87;

    const Unknown_00406f05_Body* body = reinterpret_cast<const Unknown_00406f05_Body*>(static_cast<uintptr_t>(r->edi));
    const bool negativeSide = static_cast<int32_t>(r->edx) < 0;

    int32_t fraction = fistp32(ext(GP4_GLOBAL(double, 0x00627950)) * ext(GP4_GLOBAL(double, 0x006f2b04)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fraction;

    uint32_t startA;
    uint32_t endA;
    int32_t startB;
    int32_t endB;
    int32_t startS;
    int32_t endS;
    if (negativeSide) {
        startA = static_cast<uint32_t>(body->Unknown144) << 6;
        endA = static_cast<uint32_t>(body->Unknown2A4) << 6;
        startB = static_cast<int32_t>(body->Unknown146) << 6;
        endB = static_cast<int32_t>(body->Unknown2A6) << 6;
        startS = body->Unknown100;
        endS = body->Unknown260;
    } else {
        startA = static_cast<uint32_t>(body->Unknown14A) << 6;
        endA = static_cast<uint32_t>(body->Unknown2AA) << 6;
        startB = static_cast<int32_t>(body->Unknown14C) << 6;
        endB = static_cast<int32_t>(body->Unknown2AC) << 6;
        startS = body->Unknown10C;
        endS = body->Unknown26C;
    }

    const int32_t thresholdP = static_cast<int32_t>(mulShr14(static_cast<int32_t>(endA - startA), fraction) + startA);
    GP4_GLOBAL(int32_t, 0x00627820) = thresholdP;
    const int32_t scaleQ = static_cast<int32_t>(mulShr14(endB - startB, fraction) + static_cast<uint32_t>(startB));
    GP4_GLOBAL(int32_t, 0x00627824) = scaleQ;
    const int32_t span = static_cast<int32_t>(mulShr14(endS - startS, fraction) + static_cast<uint32_t>(startS));

    fraction = fistp32(ext(GP4_GLOBAL(double, 0x00627958)) * ext(GP4_GLOBAL(double, 0x006f2b04)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fraction;

    int32_t value = static_cast<int32_t>(mulShr14(fraction, span));
    int32_t divisor;
    if (value >= thresholdP) {
        value = static_cast<int32_t>(static_cast<uint32_t>(span) - static_cast<uint32_t>(value));
        divisor = static_cast<int32_t>(static_cast<uint32_t>(span) - static_cast<uint32_t>(thresholdP));
        if (divisor > span) {
            divisor = span;
        }
        if (divisor < 1) {
            divisor = 1;
        }
        if (value < 0) {
            value = 0;
        }
        if (value > divisor) {
            value = divisor;
        }
    } else {
        divisor = thresholdP;
        if (divisor == 0) {
            divisor = 1;
        }
    }

    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(scaleQ);
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(product, divisor, &remainder);

    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = (r->eflags & ~0x8d5u) | imulFlags32(product);
}
GP4_IMPL(0x00406f05, FUN_00406f05, "regs(edx:i32, edi:ptr[0x2b0]:layout=seg_00406f05) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=frac_00627950")
