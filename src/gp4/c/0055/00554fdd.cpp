#include <gp4/gp4.h>
#include <gp4/flags.h>

// Q14 cosine-table lookup (word table 0x7c5c5c, indexed by |angle| >> 3 words).
static inline int16_t cosQ14(uint16_t angle) {
    uint16_t a = angle;
    if ((int16_t)a < 0) a = (uint16_t)(0u - a);
    const uint32_t off = (uint32_t)(a >> 2) & 0xfffeu;
    return *(const int16_t*)(GP4_ARRAY(const uint8_t, 0x007c5c5c) + off);
}

// High word of (a * b) << 2, the `imul; shl ax,1; rcl dx,1` x2 Q14 product.
static inline uint16_t mulQ14(int16_t a, int16_t b) {
    const int32_t p = (int32_t)a * (int32_t)b;
    return (uint16_t)((uint32_t)p >> 14);
}

// pushal/popal: with c = cos(A) and s = cos(0x4000 - A) of the word angle A at 0x8911f8, walks the
// word list 0x89122c up to its 0x8000 terminator and stores, for each entry B, the Q14 products
// cos(0x4000 - B) * s at 0x891236 + 2i and cos(B) * c at 0x89123e + 2i. Registers are restored;
// the flags are those of the terminating `cmp ax, 0x8000`.
// FUNCTION: GP4 0x00554fdd
void __cdecl FUN_00554fdd(gp4::Regs* r) {
    const uint16_t angle = GP4_GLOBAL(uint16_t, 0x008911f8);
    const int16_t c = cosQ14(angle);
    const int16_t s = cosQ14((uint16_t)(0x4000u - angle));

    for (uint32_t i = 0;; ++i) {
        const uint16_t b = GP4_ARRAY(uint16_t, 0x0089122c)[i];
        if (b == 0x8000u) break;
        GP4_ARRAY(uint16_t, 0x00891236)[i] = mulQ14(cosQ14((uint16_t)(0x4000u - b)), s);
        GP4_ARRAY(uint16_t, 0x0089123e)[i] = mulQ14(cosQ14(GP4_ARRAY(uint16_t, 0x0089122c)[i]), c);
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(0x8000u, 0x8000u));
}
GP4_IMPL(0x00554fdd, FUN_00554fdd, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=angles_0089122c")
