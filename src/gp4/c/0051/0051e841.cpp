#include <gp4/gp4.h>
#include <gp4/flags.h>

// Low 32 bits of a one-operand imul.
static inline uint32_t mul32(uint32_t a, uint32_t b) { return a * b; }
// sar 0xe
static inline uint32_t sar14(uint32_t v) { return static_cast<uint32_t>(static_cast<int32_t>(v) >> 14); }

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

// FUNCTION: GP4 0x0051e841
// Builds a 3x3 Q14 matrix at 0x875500 from the six Q14 terms at 0x8755a8..0x8755bc and a
// signed byte copy of each element (~ value * 15/2048) at 0x9e165c..0x9e1664.
void __cdecl FUN_0051e841(gp4::Regs* r) {
    const uint32_t a8 = GP4_GLOBAL(uint32_t, 0x008755a8);
    const uint32_t ac = GP4_GLOBAL(uint32_t, 0x008755ac);
    const uint32_t b0 = GP4_GLOBAL(uint32_t, 0x008755b0);
    const uint32_t b4 = GP4_GLOBAL(uint32_t, 0x008755b4);
    const uint32_t b8 = GP4_GLOBAL(uint32_t, 0x008755b8);
    const uint32_t bc = GP4_GLOBAL(uint32_t, 0x008755bc);

    const uint32_t s = sar14(mul32(b0, b8));   // esi
    const uint32_t t = sar14(mul32(b0, bc));   // edi

    uint32_t m[9];
    m[0] = sar14(mul32(ac, bc) + mul32(a8, s));
    m[1] = 0u - sar14(mul32(b4, b8));
    m[2] = sar14(mul32(ac, s) - mul32(a8, bc));
    m[3] = sar14(mul32(ac, b8) - mul32(a8, t));
    m[4] = sar14(mul32(b4, bc));
    const uint32_t lastEbp = mul32(a8, b8);
    m[5] = 0u - sar14(mul32(ac, t) + lastEbp);
    m[6] = sar14(mul32(a8, b4));
    m[7] = b0;
    m[8] = sar14(mul32(ac, b4));

    uint32_t* matrix = GP4_ARRAY(uint32_t, 0x00875500);
    uint8_t* bytes = GP4_ARRAY(uint8_t, 0x009e165c);
    uint32_t flags = 0u;
    uint32_t edx = 0u;
    for (int i = 0; i < 9; ++i) {
        matrix[i] = m[i];
        edx = scaleToByte(m[i], &flags);
        bytes[i] = static_cast<uint8_t>(edx);
    }

    r->eax = m[8];
    r->edx = edx;
    r->ebp = lastEbp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051e841, FUN_0051e841, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
