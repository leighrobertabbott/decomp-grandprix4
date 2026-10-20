#include <gp4/gp4.h>
#include <gp4/flags.h>

// Q14 cosine-table lookup (word table 0x7c5c5c, indexed by |angle| >> 3 words).
static inline int32_t cosQ14(uint16_t angle) {
    uint16_t a = angle;
    if ((int16_t)a < 0) a = (uint16_t)(0u - a);
    const uint32_t off = (uint32_t)(a >> 2) & 0xfffeu;
    return (int32_t)*(const int16_t*)(GP4_ARRAY(const uint8_t, 0x007c5c5c) + off);
}

// `neg eax`, then the magnitude is clamped to 0x40000 keeping the sign. *flags receives the flags
// of the last writer: `cmp eax, 0x40000` on the non-negative path, the final `neg eax` otherwise.
static inline uint32_t negClamp(int32_t q, uint32_t* flags) {
    uint32_t x = 0u - (uint32_t)q;
    if ((int32_t)x < 0) {
        uint32_t m = 0u - x;
        if (m >= 0x40000u) m = 0x40000u;
        *flags = gp4::flags::neg32(m);
        return 0u - m;
    }
    *flags = gp4::flags::sub32(x, 0x40000u);
    if (x >= 0x40000u) x = 0x40000u;
    return x;
}

// pushal/popal: with the word angles A = [0x8911f6] and B = [0x8911f8] (Q14 table lookups
// cos(A), cos(0x4000 - A), cos(B), cos(0x4000 - B), the last forced to 1 when zero) stores
// [0x954470] = (cos(B) << 14) / cos(0x4000 - B), [0x954474] = 0x10000000 / that, and the clamped,
// negated quotients cos(B) * cos(0x4000 - A) / cos(0x4000 - B) at 0x954468 and
// cos(B) * cos(A) / cos(0x4000 - B) at 0x95446c. Registers are restored; the flags are those of
// the second clamp.
// FUNCTION: GP4 0x005550b6
void __cdecl FUN_005550b6(gp4::Regs* r) {
    const uint16_t a = GP4_GLOBAL(uint16_t, 0x008911f6);
    const int32_t ca = cosQ14((uint16_t)(0x4000u - a));
    const int32_t sa = cosQ14(a);
    int32_t div = cosQ14((uint16_t)(0x4000u - GP4_GLOBAL(uint16_t, 0x008911f8)));
    if (div == 0) {
        div = 1;
    }
    const int32_t cb = cosQ14(GP4_GLOBAL(uint16_t, 0x008911f8));

    int32_t& ratio = GP4_GLOBAL(int32_t, 0x00954470);
    ratio = (cb << 14) / div;
    int32_t rem;
    GP4_GLOBAL(int32_t, 0x00954474) = gp4::idiv64((int64_t)0x10000000, ratio, &rem);

    uint32_t flags;
    GP4_GLOBAL(uint32_t, 0x00954468) = negClamp(gp4::idiv64((int64_t)cb * ca, div, &rem), &flags);
    GP4_GLOBAL(uint32_t, 0x0095446c) = negClamp(gp4::idiv64((int64_t)cb * sa, div, &rem), &flags);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005550b6, FUN_005550b6, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=angles_0089122c")
