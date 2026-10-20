#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags of a 32-bit SUB/CMP a - b.
static inline uint32_t subFlags(uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    return (a < b ? 1u : 0u)
        | parityFlag(res)
        | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u)
        | ((res & 0x80000000u) ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ res)) & 0x80000000u) ? 0x800u : 0u);
}

// Low 32 bits of (a * b) >> n (imul + shrd eax, edx, n).
static inline uint32_t mulShr(uint32_t a, uint32_t b, int n) {
    const int64_t p = (int64_t)(int32_t)a * (int64_t)(int32_t)b;
    return (uint32_t)((uint64_t)p >> n);
}

static inline uint8_t* recordAt(uint32_t p) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(p));
}

// Step one 0x160-byte record through the ring at 0x008943a0: at the lap end
// [0x00866ecc] wrap back by [0x00866ec4] (below [0x00891500]) or clamp to
// [0x00866ef8] - 0x160 (at or past it). *fl gets the last CMP/SUB flags.
static inline uint32_t advance(uint32_t p, uint32_t* fl) {
    p += 0x160u;
    const uint32_t lapEnd = GP4_GLOBAL(uint32_t, 0x00866ecc);
    if (p < lapEnd) {
        *fl = subFlags(p, lapEnd);
        return p;
    }
    const uint32_t wrapLimit = GP4_GLOBAL(uint32_t, 0x00891500);
    if (p < wrapLimit) {
        const uint32_t lapSize = GP4_GLOBAL(uint32_t, 0x00866ec4);
        *fl = subFlags(p, lapSize);
        return p - lapSize;
    }
    const uint32_t clampEnd = GP4_GLOBAL(uint32_t, 0x00866ef8);
    if (p < clampEnd) {
        *fl = subFlags(p, clampEnd);
        return p;
    }
    *fl = subFlags(clampEnd, 0x160u);
    return clampEnd - 0x160u;
}

// byte +0x8c = value; bit 1 of byte +0x73 follows the byte flag [0x00627b65].
static inline void markRecord(uint32_t p, uint8_t value) {
    uint8_t* const rec = recordAt(p);
    rec[0x8c] = value;
    if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0) {
        rec[0x73] |= 2u;
    } else {
        rec[0x73] &= 0xfdu;
    }
}

static inline int32_t clampByte(int32_t v) {
    if (v > 0x7f) v = 0x7f;
    if (v < -0x80) v = -0x80;
    return v;
}

// FUNCTION: GP4 0x0043f696
void __cdecl FUN_0043f696(gp4::Regs* r) {
    uint32_t fl = 0;

    uint32_t mag = GP4_GLOBAL(uint32_t, 0x00627b30);
    if ((int32_t)mag < 0) mag = 0u - mag;
    if ((int32_t)mag > 0x6000) mag = 0x6000u;
    const uint32_t b50 = GP4_GLOBAL(uint32_t, 0x00627b50);
    const uint32_t scaled = mulShr(0x6000u - mag, GP4_GLOBAL(uint32_t, 0x006247e8), 14);
    uint32_t n1 = b50 - mulShr(scaled, b50, 14);
    if ((int32_t)n1 < 0) n1 = 0;

    uint32_t p = GP4_GLOBAL(uint32_t, 0x00627b48);
    if (n1 != 0) {
        uint32_t i = 0;
        uint32_t c = n1;
        do {
            int32_t rem;
            int32_t v = (gp4::idiv64((int64_t)(int32_t)i << 14, (int32_t)n1, &rem) - 0x4000) >> 8;
            if (v == 0) v = 1;
            markRecord(p, (uint8_t)v);
            p = advance(p, &fl);
            ++i;
        } while (--c != 0);
    }

    for (uint32_t c = GP4_GLOBAL(uint32_t, 0x00627b58); c != 0; --c) {
        markRecord(p, 1);
        p = advance(p, &fl);
    }

    const uint32_t rest = GP4_GLOBAL(uint32_t, 0x0062ac48) - n1;
    const uint32_t b58 = GP4_GLOBAL(uint32_t, 0x00627b58);
    const uint32_t n2 = rest - b58;
    if ((int32_t)n2 < 0) {
        fl = subFlags(rest, b58);
        r->eflags = (r->eflags & ~0x8d5u) | fl;
        return;
    }
    {
        uint32_t i = 0;
        uint32_t c = n2 + 1u;
        do {
            int32_t rem;
            int32_t v = gp4::idiv64((int64_t)(int32_t)i << 14, (int32_t)n2, &rem) >> 8;
            if (v == 0) v = 1;
            markRecord(p, (uint8_t)v);
            p = advance(p, &fl);
            ++i;
        } while (--c != 0);
    }

    if (GP4_GLOBAL(uint32_t, 0x00627b58) == 0) {
        uint32_t c = GP4_GLOBAL(uint32_t, 0x0062ac48);
        if (c == 0) {
            fl = 0x44u;  // or ecx, ecx with ecx == 0
        } else {
            uint32_t q = GP4_GLOBAL(uint32_t, 0x00627b48);
            int32_t v = GP4_GLOBAL(int32_t, 0x00627b50);
            do {
                v = clampByte(v);
                recordAt(q)[0x135] = (uint8_t)v;
                --v;
                q = advance(q, &fl);
            } while (--c != 0);
        }
    } else {
        uint32_t c = (GP4_GLOBAL(uint32_t, 0x00627b58) >> 1) + GP4_GLOBAL(uint32_t, 0x00627b50);
        if (c == 0) {
            fl = 0x44u;  // or ecx, ecx with ecx == 0
        } else {
            uint32_t left = GP4_GLOBAL(uint32_t, 0x0062ac48);
            uint32_t q = GP4_GLOBAL(uint32_t, 0x00627b48);
            int32_t v = GP4_GLOBAL(int32_t, 0x00627b50);
            do {
                v = clampByte(v);
                recordAt(q)[0x135] = (uint8_t)v;
                --v;
                q = advance(q, &fl);
                --left;
            } while (--c != 0);
            fl = subFlags(left, 0u);  // cmp ecx, 0
            if ((int32_t)left > 0) {
                v = (int32_t)(left - GP4_GLOBAL(uint32_t, 0x00627b54));
                c = left;
                do {
                    v = clampByte(v);
                    recordAt(q)[0x135] = (uint8_t)v;
                    --v;
                    q = advance(q, &fl);
                } while (--c != 0);
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | fl;
}
GP4_IMPL(0x0043f696, FUN_0043f696, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_0043f696")
