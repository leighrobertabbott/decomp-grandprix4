#include <gp4/gp4.h>
#include <gp4/x87.h>

// Four 0x300-byte records at 0x006380d8 (only the fields this routine touches).
struct Unknown_006380d8 {
    uint8_t Unknown000[0x14];
    int32_t Unknown014;
    uint8_t Unknown018[0x38 - 0x18];
    int32_t Unknown038;
    uint8_t Unknown03c[0x240 - 0x3c];
    int32_t Unknown240;
    int32_t Unknown244;
    int32_t Unknown248;
    uint8_t Unknown24c[0x300 - 0x24c];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags of a 32-bit SUB/CMP a - b (NEG v is 0 - v).
static inline uint32_t subFlags(uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    return (a < b ? 1u : 0u)
        | parityFlag(res)
        | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u)
        | ((res & 0x80000000u) ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ res)) & 0x80000000u) ? 0x800u : 0u);
}

// Low 32 bits of (a * b) >> 12 (imul + shrd eax, edx, 12).
static inline uint32_t mulShr12(uint32_t a, int32_t b) {
    const int64_t p = (int64_t)(int32_t)a * (int64_t)b;
    return (uint32_t)((uint64_t)p >> 12);
}

// Symmetric clamp to [-lim, lim] (unsigned compares on the magnitude);
// *flags receives the flags of the last CMP/NEG the original executes.
static inline uint32_t clampSym(uint32_t x, uint32_t lim, uint32_t* flags) {
    if ((int32_t)x < 0) {
        uint32_t m = 0u - x;
        if (m >= lim) m = lim;
        *flags = subFlags(0u, m);
        return 0u - m;
    }
    *flags = subFlags(x, lim);
    return (x >= lim) ? lim : x;
}

// FILD of (Unknown038 + bias) times three table coefficients, each FISTP'd
// straight from the 80-bit product.
static inline void project(Unknown_006380d8* rec, int32_t bias, const double* table) {
    GP4_GLOBAL(int32_t, 0x006f2bf4) = rec->Unknown038 + bias;
    const gp4::x87::Ext v = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    rec->Unknown240 = gp4::x87::fistp32(v * gp4::x87::ext(table[0x60 / 8]));
    rec->Unknown244 = gp4::x87::fistp32(v * gp4::x87::ext(table[0x78 / 8]));
    rec->Unknown248 = gp4::x87::fistp32(v * gp4::x87::ext(table[0x90 / 8]));
}

// FUNCTION: GP4 0x0041fdd5
void __cdecl FUN_0041fdd5(gp4::Regs* r) {
    Unknown_006380d8* const rec = GP4_ARRAY(Unknown_006380d8, 0x006380d8);
    const double* const table = GP4_ARRAY(const double, 0x00622e98);

    if ((rec[0].Unknown014 | rec[1].Unknown014 | rec[2].Unknown014 | rec[3].Unknown014) != 0) {
        gp4::Regs sub = *r;
        gp4::call_regs(0x0041fbef, sub);
    } else {
        GP4_GLOBAL(uint32_t, 0x00622900) = 0;
        GP4_GLOBAL(uint32_t, 0x00622904) = 0;
        GP4_GLOBAL(uint32_t, 0x00622908) = 0;
    }

    const int32_t biasA = GP4_GLOBAL(int32_t, 0x00637f40) >> 1;
    project(&rec[0], biasA, table);
    project(&rec[1], biasA, table);
    const int32_t biasB = GP4_GLOBAL(int32_t, 0x00637f44) >> 1;
    project(&rec[2], biasB, table);
    project(&rec[3], biasB, table);

    const uint32_t x0 = (uint32_t)rec[0].Unknown240, x1 = (uint32_t)rec[1].Unknown240;
    const uint32_t x2 = (uint32_t)rec[2].Unknown240, x3 = (uint32_t)rec[3].Unknown240;
    const uint32_t y0 = (uint32_t)rec[0].Unknown244, y1 = (uint32_t)rec[1].Unknown244;
    const uint32_t y2 = (uint32_t)rec[2].Unknown244, y3 = (uint32_t)rec[3].Unknown244;
    const uint32_t z0 = (uint32_t)rec[0].Unknown248, z1 = (uint32_t)rec[1].Unknown248;
    const uint32_t z2 = (uint32_t)rec[2].Unknown248, z3 = (uint32_t)rec[3].Unknown248;

    uint32_t flags = 0;

    const uint32_t ebp1 = mulShr12(y0 - y1, GP4_GLOBAL(int32_t, 0x00638d7c))
                        + mulShr12(y2 - y3, GP4_GLOBAL(int32_t, 0x00638d80));
    const uint32_t s1 = (0u - mulShr12(x0 + x1 + x2 + x3, GP4_GLOBAL(int32_t, 0x00638d88)))
                      + ebp1 + GP4_GLOBAL(uint32_t, 0x00622908);
    GP4_GLOBAL(uint32_t, 0x006228fc) = clampSym(s1, GP4_GLOBAL(uint32_t, 0x006224b1), &flags);

    const uint32_t ebp2 = (0u - mulShr12(y0 + y1, GP4_GLOBAL(int32_t, 0x00638d74)))
                        + mulShr12(y2 + y3, GP4_GLOBAL(int32_t, 0x00638d78));
    const uint32_t s2 = mulShr12(z0 + z1 + z2 + z3, GP4_GLOBAL(int32_t, 0x00638d8c))
                      + ebp2 + GP4_GLOBAL(uint32_t, 0x00622900);
    GP4_GLOBAL(uint32_t, 0x006228f4) = clampSym(s2, GP4_GLOBAL(uint32_t, 0x006224ad), &flags);

    const uint32_t ebp3 = mulShr12(x1 + x0, GP4_GLOBAL(int32_t, 0x00638d6c))
                        - mulShr12(x3 + x2, GP4_GLOBAL(int32_t, 0x00638d70));
    const int64_t last = (int64_t)(int32_t)(z1 - z0 + z3 - z2)
                       * (int64_t)GP4_GLOBAL(int32_t, 0x00638d84);
    const uint32_t s3 = (uint32_t)((uint64_t)last >> 12) + ebp3 + GP4_GLOBAL(uint32_t, 0x00622904);
    const uint32_t out3 = clampSym(s3, GP4_GLOBAL(uint32_t, 0x006224b5), &flags);
    GP4_GLOBAL(uint32_t, 0x006228f8) = out3;

    r->eax = out3;
    r->ebx = 0x006380d8u;
    r->ecx = (uint32_t)biasB;
    r->edx = (uint32_t)((uint64_t)last >> 32);
    r->ebp = ebp3;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041fdd5, FUN_0041fdd5, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=recs_006380d8")
