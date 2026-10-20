#include <gp4/gp4.h>
#include <gp4/flags.h>

// Low 32 bits of a one-operand imul.
static inline uint32_t mul32(uint32_t a, uint32_t b) { return a * b; }
// sar 0xe
static inline uint32_t sar14(uint32_t v) { return static_cast<uint32_t>(static_cast<int32_t>(v) >> 14); }
// imul; shrd eax, edx, 0xe: low 32 bits of the signed 64-bit product shifted right by 14.
static inline uint32_t mulShr14(uint32_t a, uint32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(a)) *
                                                       static_cast<int32_t>(b)) >> 14);
}

// Signed byte d = sign(x) * ((|x| - (|x| >> 4)) >> 7). Returns the full edx value and the
// flags of the last flag writer (`neg edx` for x < 0, else `or eax,eax`).
static inline uint32_t scaleToByte(uint32_t x, uint32_t* flagsOut) {
    const bool negative = (x & 0x80000000u) != 0u;
    const uint32_t mag = negative ? 0u - x : x;
    uint32_t d = (mag - (mag >> 4)) >> 7;
    if (negative) {
        *flagsOut = gp4::flags::neg32(d);
        d = 0u - d;
    } else {
        *flagsOut = gp4::flags::logic32(x);
    }
    return d;
}

// FUNCTION: GP4 0x0051e5b2
// Builds a 3x3 Q14 matrix at 0x8752b8..0x8752b4 from the five Q14 terms at 0x8755d8..0x8755ec,
// a signed byte copy of each element (~ value * 15/2048) at 0x8752d0..0x8752d8, and the first six
// elements scaled by [0x875200] (three) and [0x875204] (three) into 0x875294..0x8752a8.
// Same scheme as 0x0051e841, with 64-bit products for the two shared terms.
void __cdecl FUN_0051e5b2(gp4::Regs* r) {
    const uint32_t d8 = GP4_GLOBAL(uint32_t, 0x008755d8);
    const uint32_t dc = GP4_GLOBAL(uint32_t, 0x008755dc);
    const uint32_t e0 = GP4_GLOBAL(uint32_t, 0x008755e0);
    const uint32_t e4 = GP4_GLOBAL(uint32_t, 0x008755e4);
    const uint32_t e8 = GP4_GLOBAL(uint32_t, 0x008755e8);
    const uint32_t ec = GP4_GLOBAL(uint32_t, 0x008755ec);

    const uint32_t s = mulShr14(e0, e8);   // esi
    const uint32_t t = mulShr14(e0, ec);   // edi

    uint32_t m[9];
    m[0] = sar14(mul32(dc, ec) + mul32(d8, s));
    m[1] = 0u - sar14(mul32(e4, e8));
    m[2] = sar14(mul32(dc, s) - mul32(d8, ec));
    m[3] = sar14(mul32(dc, e8) - mul32(d8, t));
    m[4] = sar14(mul32(e4, ec));
    const uint32_t lastEbp = mul32(d8, e8);
    m[5] = 0u - sar14(mul32(dc, t) + lastEbp);
    m[6] = sar14(mul32(d8, e4));
    m[7] = e0;
    m[8] = sar14(mul32(dc, e4));

    // Store order of the original: elements 0..5 at 0x8752b8.., then 6, 7, 8 at 0x8752ac..0x8752b4.
    static const uint32_t kSlot[9] = {0x8752b8u, 0x8752bcu, 0x8752c0u, 0x8752c4u, 0x8752c8u,
                                      0x8752ccu, 0x8752acu, 0x8752b0u, 0x8752b4u};
    (void)kSlot;
    uint32_t* const matrix = GP4_ARRAY(uint32_t, 0x008752ac);   // [0..2] = m6..m8, [3..8] = m0..m5
    uint8_t* const bytes = GP4_ARRAY(uint8_t, 0x008752d0);
    uint32_t* const scaled = GP4_ARRAY(uint32_t, 0x00875294);
    const uint32_t scaleA = GP4_GLOBAL(uint32_t, 0x00875200);
    const uint32_t scaleB = GP4_GLOBAL(uint32_t, 0x00875204);

    uint32_t flags = 0u;
    uint32_t edx = 0u;
    for (int i = 0; i < 9; ++i) {
        if (i < 6) {
            matrix[i + 3] = m[i];
        } else {
            matrix[i - 6] = m[i];
        }
        edx = scaleToByte(m[i], &flags);
        bytes[i] = static_cast<uint8_t>(edx);
        if (i < 6) {
            scaled[i] = mulShr14(m[i], i < 3 ? scaleA : scaleB);
        }
    }

    r->eax = m[8];
    r->edx = edx;
    r->ebp = lastEbp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051e5b2, FUN_0051e5b2, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
