#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// flags of "cmp a, b" / "sub a, b" at 32-bit width
static inline uint32_t subFlags32(uint32_t a, uint32_t b) {
    const uint32_t r = a - b;
    return parityFlag(r)
        | (a < b ? 0x01u : 0u)
        | ((((a ^ b ^ r) & 0x10u) != 0u) ? 0x10u : 0u)
        | (r == 0u ? 0x40u : 0u)
        | ((r >> 31) != 0u ? 0x80u : 0u)
        | (((((a ^ b) & (a ^ r)) >> 31) != 0u) ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00421fc3
void __cdecl FUN_00421fc3(gp4::Regs* r) {
    const int32_t limit = GP4_GLOBAL(int32_t, 0x00608104);
    const uint32_t sumLo = GP4_GLOBAL(uint32_t, 0x006080fc);
    int32_t sumHi = GP4_GLOBAL(int32_t, 0x00608100);

    // (sumHi:sumLo) * 2 with sumHi clamped, then the high word clamped to the divisor
    if (sumHi > 0x3fffffff) sumHi = 0x3fffffff;
    uint32_t top = (static_cast<uint32_t>(sumHi) << 1) | (sumLo >> 31);
    if (static_cast<int32_t>(top) > limit) top = static_cast<uint32_t>(limit);
    const uint32_t dividendLo = (sumLo & 0x7fffffffu) | (top << 31);
    const int32_t dividendHi = static_cast<int32_t>(top) >> 1;

    // flags left by "cmp ebp, 1" survive the idiv
    int32_t divisor = limit;
    uint32_t flags = subFlags32(static_cast<uint32_t>(limit), 1u);
    if (limit < 1) divisor = 1;

    int32_t rem;
    int32_t quot = gp4::idiv64((static_cast<int64_t>(dividendHi) << 32) | dividendLo, divisor, &rem);
    GP4_GLOBAL(int32_t, 0x00608108) = quot;

    const int32_t previous = GP4_GLOBAL(int32_t, 0x0060810c);
    flags = subFlags32(static_cast<uint32_t>(quot), static_cast<uint32_t>(previous));
    uint32_t ebp = static_cast<uint32_t>(quot);
    uint32_t eax = static_cast<uint32_t>(quot);
    uint32_t edx = static_cast<uint32_t>(rem);

    if (quot > previous) {
        GP4_GLOBAL(int32_t, 0x0060810c) = quot;
        flags = subFlags32(static_cast<uint32_t>(quot), 1u);
        const int32_t div2 = quot < 1 ? 1 : quot;
        ebp = static_cast<uint32_t>(div2);

        int32_t rem2;
        const int64_t a = (static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00608118)) << 32) | GP4_GLOBAL(uint32_t, 0x00608114);
        GP4_GLOBAL(int32_t, 0x0060812c) = gp4::idiv64(a, div2, &rem2);
        const int64_t b = (static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00608120)) << 32) | GP4_GLOBAL(uint32_t, 0x0060811c);
        GP4_GLOBAL(int32_t, 0x00608130) = gp4::idiv64(b, div2, &rem2);
        const int64_t c = (static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00608128)) << 32) | GP4_GLOBAL(uint32_t, 0x00608124);
        eax = static_cast<uint32_t>(gp4::idiv64(c, div2, &rem2));
        GP4_GLOBAL(int32_t, 0x00608134) = static_cast<int32_t>(eax);
        edx = static_cast<uint32_t>(rem2);
    }

    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00421fc3, FUN_00421fc3, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00421fc3+pairs_00421fc3")
