#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Clamp the magnitude of a signed value to limit (unsigned compare, as "jb").
static inline uint32_t clampMagnitude(uint32_t v, uint32_t limit) {
    if ((int32_t)v < 0) {
        uint32_t m = 0u - v;
        if (m >= limit) m = limit;
        return 0u - m;
    }
    if (v >= limit) v = limit;
    return v;
}

// "imul m32; idiv m32": (a * b) / divisor truncating toward zero, remainder with
// the dividend's sign. The original faults when the quotient does not fit; that
// case is not modelled.
static inline int32_t mulDiv(int32_t a, int32_t b, int32_t divisor, uint32_t* remainderOut) {
    const int64_t dividend = (int64_t)a * (int64_t)b;
    const bool negativeDividend = dividend < 0;
    const bool negativeDivisor = divisor < 0;
    const uint64_t magnitude = negativeDividend ? 0ull - (uint64_t)dividend : (uint64_t)dividend;
    const uint32_t d = negativeDivisor ? 0u - (uint32_t)divisor : (uint32_t)divisor;
    uint32_t remainder = (uint32_t)(magnitude >> 32);
    uint32_t low = (uint32_t)magnitude;
    uint32_t quotient = 0;
    for (int bit = 0; bit < 32; bit++) {
        const uint32_t carry = remainder >> 31;
        remainder = (remainder << 1) | (low >> 31);
        low <<= 1;
        quotient <<= 1;
        if (carry != 0u || remainder >= d) {
            remainder -= d;
            quotient |= 1u;
        }
    }
    *remainderOut = negativeDividend ? 0u - remainder : remainder;
    return (int32_t)((negativeDividend != negativeDivisor) ? 0u - quotient : quotient);
}

static inline int32_t atLeastZero(int32_t v) { return v < 0 ? 0 : v; }

// FUNCTION: GP4 0x00422056
void __cdecl FUN_00422056(gp4::Regs* r) {
    uint8_t* const owner = (uint8_t*)r->esi;
    const int32_t total = GP4_GLOBAL(int32_t, 0x0060810c);

    if (total == (int32_t)0x80000000) {
        // cmp [0x60810c], 0x80000000 found equal
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }
    if (total == 0) {
        GP4_FIELD(uint32_t, owner, 0x2ec) = 0;
        GP4_FIELD(uint32_t, owner, 0x2f0) = 0;
        GP4_FIELD(uint32_t, owner, 0x2f4) = 0;
        GP4_FIELD(uint32_t, owner, 0x2f8) = 0;
        r->eax = 0;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    uint32_t remainder;

    const int32_t d334 = GP4_GLOBAL(int32_t, 0x00601334);
    const uint32_t a1 = clampMagnitude(GP4_GLOBAL(uint32_t, 0x00608134) + GP4_GLOBAL(uint32_t, 0x0060134c),
                                       (uint32_t)d334 << 1);
    GP4_GLOBAL(int32_t, 0x00608138) = mulDiv((int32_t)a1, total, d334, &remainder);
    GP4_GLOBAL(int32_t, 0x0060813c) =
        atLeastZero((int32_t)((uint32_t)total - (uint32_t)GP4_GLOBAL(int32_t, 0x00608138)));

    const int32_t d340 = GP4_GLOBAL(int32_t, 0x00601340);
    const uint32_t a2 = clampMagnitude(GP4_GLOBAL(uint32_t, 0x0060812c) + GP4_GLOBAL(uint32_t, 0x00601344),
                                       (uint32_t)d340 << 1);
    GP4_GLOBAL(int32_t, 0x0060814c) =
        atLeastZero(mulDiv((int32_t)a2, GP4_GLOBAL(int32_t, 0x00608138), d340, &remainder));
    GP4_GLOBAL(int32_t, 0x00608148) = atLeastZero(
        (int32_t)((uint32_t)GP4_GLOBAL(int32_t, 0x00608138) - (uint32_t)GP4_GLOBAL(int32_t, 0x0060814c)));

    const int32_t d338 = GP4_GLOBAL(int32_t, 0x00601338);
    const uint32_t a3 = clampMagnitude(GP4_GLOBAL(uint32_t, 0x0060812c) + GP4_GLOBAL(uint32_t, 0x0060133c),
                                       (uint32_t)d338 << 1);
    GP4_GLOBAL(int32_t, 0x00608144) =
        atLeastZero(mulDiv((int32_t)a3, GP4_GLOBAL(int32_t, 0x0060813c), d338, &remainder));
    GP4_GLOBAL(int32_t, 0x00608140) = atLeastZero(
        (int32_t)((uint32_t)GP4_GLOBAL(int32_t, 0x0060813c) - (uint32_t)GP4_GLOBAL(int32_t, 0x00608144)));

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00608030);
    GP4_FIELD(uint32_t, owner, 0x2ec) = clampMagnitude(GP4_GLOBAL(uint32_t, 0x00608140), limit) << 4;
    GP4_FIELD(uint32_t, owner, 0x2f0) = clampMagnitude(GP4_GLOBAL(uint32_t, 0x00608144), limit) << 4;
    GP4_FIELD(uint32_t, owner, 0x2f4) = clampMagnitude(GP4_GLOBAL(uint32_t, 0x00608148), limit) << 4;
    const uint32_t last = clampMagnitude(GP4_GLOBAL(uint32_t, 0x0060814c), limit);
    const uint32_t shifted = last << 4;
    GP4_FIELD(uint32_t, owner, 0x2f8) = shifted;

    // Flags of the final "shl eax, 4" (pinned oracle model: CF/OF from the
    // value shifted by 3, AF clear).
    const uint32_t previous = last << 3;
    const uint32_t flags = (previous >> 31)
        | parityFlag(shifted)
        | (shifted == 0u ? 0x40u : 0u)
        | ((shifted >> 31) << 7)
        | ((((previous ^ shifted) >> 31) & 1u) << 11);

    r->eax = shifted;
    r->edx = remainder;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00422056, FUN_00422056, "regs(esi:ptr[0x300]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00422056")
