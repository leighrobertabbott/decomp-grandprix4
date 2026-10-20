#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

// Result of an x87 compare (FTST / FUCOMP / FUCOMPP) on extended operands:
// C0 = less-or-unordered, C3 = equal-or-unordered, C2 = unordered.
enum { kLess = 0, kGreater = 1, kEqual = 2, kUnordered = 3 };

// Exact comparison of two 80-bit values by their bit patterns (sign, exponent,
// 64-bit mantissa as two halves); no rounding is involved.
static inline int extCompare(const Ext& a, const Ext& b) {
    const uint32_t aLo = a.b[0] | (a.b[1] << 8) | (a.b[2] << 16) | ((uint32_t)a.b[3] << 24);
    const uint32_t aHi = a.b[4] | (a.b[5] << 8) | (a.b[6] << 16) | ((uint32_t)a.b[7] << 24);
    const uint32_t aSe = a.b[8] | (a.b[9] << 8);
    const uint32_t bLo = b.b[0] | (b.b[1] << 8) | (b.b[2] << 16) | ((uint32_t)b.b[3] << 24);
    const uint32_t bHi = b.b[4] | (b.b[5] << 8) | (b.b[6] << 16) | ((uint32_t)b.b[7] << 24);
    const uint32_t bSe = b.b[8] | (b.b[9] << 8);
    const uint32_t aExp = aSe & 0x7fffu;
    const uint32_t bExp = bSe & 0x7fffu;

    if (aExp == 0x7fffu && ((aHi & 0x7fffffffu) | aLo) != 0u) return kUnordered;
    if (bExp == 0x7fffu && ((bHi & 0x7fffffffu) | bLo) != 0u) return kUnordered;

    const bool aZero = (aExp == 0u) && (aHi | aLo) == 0u;
    const bool bZero = (bExp == 0u) && (bHi | bLo) == 0u;
    if (aZero && bZero) return kEqual;

    const uint32_t aNeg = aZero ? 0u : (aSe >> 15);
    const uint32_t bNeg = bZero ? 0u : (bSe >> 15);
    if (aNeg != bNeg) return aNeg ? kLess : kGreater;

    int mag;   // magnitude order of a against b
    if (aExp != bExp) mag = (aExp < bExp) ? kLess : kGreater;
    else if (aHi != bHi) mag = (aHi < bHi) ? kLess : kGreater;
    else if (aLo != bLo) mag = (aLo < bLo) ? kLess : kGreater;
    else mag = kEqual;
    if (mag == kEqual || !aNeg) return mag;
    return (mag == kLess) ? kGreater : kLess;
}

// FABS on an extended value: clear the sign bit.
static inline Ext extAbs(const Ext& v) {
    Ext r = v;
    r.b[9] = (uint8_t)(r.b[9] & 0x7f);
    return r;
}

// Flags left by `fnstsw ax; sahf`: C0 -> CF, C2 -> PF, TOP bit 1 (status bit 12)
// -> AF, C3 -> ZF, busy bit (always 0) -> SF. `depth` is the x87 stack depth
// when the status word is read, TOP = (8 - depth) & 7.
static inline uint32_t sahfFlags(int cmp, int depth) {
    uint32_t flags = 0;
    if (cmp == kLess || cmp == kUnordered) flags |= 0x001u;
    if (cmp == kUnordered) flags |= 0x004u;
    if ((((8 - depth) & 7) & 2) != 0) flags |= 0x010u;
    if (cmp == kEqual || cmp == kUnordered) flags |= 0x040u;
    return flags;
}

// FUNCTION: GP4 0x00401d08
// Entry stack: st0 = limit, st1 = numerator, st2 = denominator. Leaves
// st0 = numerator/denominator unless its magnitude reaches the limit, in which
// case it is +-limit with the sign of the quotient; a zero denominator gives
// +-limit by the sign of the numerator. The caller's eax is preserved.
void __cdecl FUN_00401d08(gp4::Regs* r) {
    const Ext limit = gp4::x87::in_st(r, 0);
    const Ext numerator = gp4::x87::in_st(r, 1);
    const Ext denominator = gp4::x87::in_st(r, 2);
    const int depth = r->st_in;
    const Ext zero = gp4::x87::ext(0.0);

    int cmp = extCompare(denominator, zero);              // ftst
    uint32_t flags = sahfFlags(cmp, depth);
    Ext result;
    int signCmp;                                           // compared against zero for the sign
    bool clamp;

    if (cmp == kLess || cmp == kGreater) {                 // jne: denominator != 0
        const Ext quotient = numerator / denominator;      // fdivp
        cmp = extCompare(extAbs(quotient), limit);         // fucomp st(2)
        flags = sahfFlags(cmp, depth - 1);
        clamp = !(cmp == kLess || cmp == kUnordered);      // jb not taken
        result = quotient;
        signCmp = clamp ? extCompare(quotient, zero) : kGreater;
        if (clamp) flags = sahfFlags(signCmp, depth - 2);
    } else {                                               // zero (or unordered) denominator
        signCmp = extCompare(numerator, zero);             // fucompp
        flags = sahfFlags(signCmp, depth - 2);
        clamp = true;
    }

    if (clamp) {
        // fld1; jae -> keep +1, else fchs; fmulp st(1)
        const bool negate = (signCmp == kLess || signCmp == kUnordered);
        const Ext sign = gp4::x87::ext(negate ? -1.0 : 1.0);
        result = limit * sign;
    }

    gp4::x87::set_out_st(r, 0, result);
    r->eflags = (r->eflags & ~0xd5u) | flags;
}
GP4_IMPL(0x00401d08, FUN_00401d08, "regs(st0:f64, st1:f64, st2:f64) -> (st0, cf, pf, af, zf, sf)")
