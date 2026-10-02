#include <gp4/gp4.h>
#include <gp4/x87.h>

// Four 0x300-byte records at 0x006380d8 (only the fields this routine touches).
struct Unknown_006380d8 {
    uint8_t Unknown000[0x14];
    int32_t Unknown014;
    uint8_t Unknown018[0x200 - 0x18];
    int32_t Unknown200;
    int32_t Unknown204;
    int32_t Unknown208;
    uint8_t Unknown20c[0x300 - 0x20c];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

// FISTP applied directly to an 80-bit product (no intermediate double rounding):
// adding and removing 1.5 * 2^63 at extended precision rounds to an integer with
// the current rounding mode; the integral result then converts exactly (or
// overflows to the integer indefinite exactly as the original's FISTP does).
static inline int32_t fistpExt32(const gp4::x87::Ext& x) {
    const gp4::x87::Ext magic = gp4::x87::ext(13835058055282163712.0);
    const gp4::x87::Ext rounded = (x + magic) - magic;
    return gp4::x87::fistp32(gp4::x87::narrow(rounded));
}

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Low 32 bits of (a * b) >> 12 (imul + shrd eax, edx, 12).
static inline uint32_t mulShr12(uint32_t a, int32_t b) {
    const int64_t p = (int64_t)(int32_t)a * (int64_t)b;
    return (uint32_t)((uint64_t)p >> 12);
}

// FUNCTION: GP4 0x0041fbef
void __cdecl FUN_0041fbef(gp4::Regs* r) {
    Unknown_006380d8* const rec = GP4_ARRAY(Unknown_006380d8, 0x006380d8);
    const double* const table = GP4_ARRAY(const double, 0x00622e98);

    for (int k = 0; k < 4; k++) {
        GP4_GLOBAL(int32_t, 0x006f2bf4) = rec[k].Unknown014;
        const gp4::x87::Ext v = gp4::x87::ext((double)GP4_GLOBAL(int32_t, 0x006f2bf4));
        rec[k].Unknown200 = fistpExt32(v * gp4::x87::ext(table[0x60 / 8]));
        rec[k].Unknown204 = fistpExt32(v * gp4::x87::ext(table[0x78 / 8]));
        rec[k].Unknown208 = fistpExt32(v * gp4::x87::ext(table[0x90 / 8]));
    }

    const uint32_t x0 = (uint32_t)rec[0].Unknown200, x1 = (uint32_t)rec[1].Unknown200;
    const uint32_t x2 = (uint32_t)rec[2].Unknown200, x3 = (uint32_t)rec[3].Unknown200;
    const uint32_t y0 = (uint32_t)rec[0].Unknown204, y1 = (uint32_t)rec[1].Unknown204;
    const uint32_t y2 = (uint32_t)rec[2].Unknown204, y3 = (uint32_t)rec[3].Unknown204;
    const uint32_t z0 = (uint32_t)rec[0].Unknown208, z1 = (uint32_t)rec[1].Unknown208;
    const uint32_t z2 = (uint32_t)rec[2].Unknown208, z3 = (uint32_t)rec[3].Unknown208;

    GP4_GLOBAL(uint32_t, 0x00622908) =
        mulShr12(y0 - y1 + y2 - y3, GP4_GLOBAL(int32_t, 0x00638d94))
        - mulShr12(x0 + x1 + x2 + x3, GP4_GLOBAL(int32_t, 0x00638d88));

    GP4_GLOBAL(uint32_t, 0x00622900) =
        mulShr12(z0 + z1 + z2 + z3, GP4_GLOBAL(int32_t, 0x00638d8c))
        + (mulShr12(y2 + y3, GP4_GLOBAL(int32_t, 0x00638d78))
           - mulShr12(y0 + y1, GP4_GLOBAL(int32_t, 0x00638d74)));

    const uint32_t ebp = mulShr12(x1 + x0, GP4_GLOBAL(int32_t, 0x00638d6c))
                       - mulShr12(x3 + x2, GP4_GLOBAL(int32_t, 0x00638d70));
    const int64_t last = (int64_t)(int32_t)(z1 - z0 + z3 - z2)
                       * (int64_t)GP4_GLOBAL(int32_t, 0x00638d90);
    const uint32_t a = (uint32_t)((uint64_t)last >> 12);
    const uint32_t sum = a + ebp;
    GP4_GLOBAL(uint32_t, 0x00622904) = sum;

    const uint32_t flags = (sum < a ? 1u : 0u)
        | parityFlag(sum)
        | ((a ^ ebp ^ sum) & 0x10u)
        | (sum == 0u ? 0x40u : 0u)
        | ((sum & 0x80000000u) ? 0x80u : 0u)
        | (((~(a ^ ebp) & (a ^ sum)) & 0x80000000u) ? 0x800u : 0u);

    r->eax = sum;
    r->ebx = 0x006380d8u;
    r->edx = (uint32_t)((uint64_t)last >> 32);
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041fbef, FUN_0041fbef, "regs() -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of)")
