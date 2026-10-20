#include <gp4/gp4.h>
#include <gp4/x87.h>

// Four 0x300-byte records at 0x006380d8 (only the fields this routine touches).
struct Unknown_006380d8 {
    uint8_t Unknown000[0x38];
    uint32_t Unknown038;
    uint8_t Unknown03c[0xc8 - 0x3c];
    uint32_t Unknown0c8;
    uint8_t Unknown0cc[0x114 - 0xcc];
    int32_t Unknown114[16];               // +0x114 .. +0x150
    uint8_t Unknown154[0x300 - 0x154];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

// FISTP applied directly to an 80-bit value (no intermediate double rounding):
// adding and removing 1.5 * 2^63 at extended precision rounds to an integer with
// the current rounding mode; the integral result then converts exactly (or
// overflows to the integer indefinite exactly as the original's FISTP does).
static inline int32_t fistpExt32(const gp4::x87::Ext& x) {
    const gp4::x87::Ext magic = gp4::x87::ext(13835058055282163712.0);
    const gp4::x87::Ext rounded = (x + magic) - magic;
    return gp4::x87::fistp32(gp4::x87::narrow(rounded));
}


static inline gp4::x87::Ext extInt(int32_t v) { return gp4::x87::ext((double)v); }

// f[0]*f[2] + f[4]*f[6] + f[1]*f[3] + f[5]*f[7], kept in extended precision.
static inline gp4::x87::Ext dot4(const int32_t* f) {
    gp4::x87::Ext sum = extInt(f[0]) * extInt(f[2]);
    sum = sum + extInt(f[4]) * extInt(f[6]);
    sum = sum + extInt(f[1]) * extInt(f[3]);
    sum = sum + extInt(f[5]) * extInt(f[7]);
    return sum;
}

// Clamp the magnitude of a signed value to limit (unsigned compare).
static inline uint32_t clampMagnitude(uint32_t v, uint32_t limit) {
    if ((int32_t)v < 0) {
        uint32_t m = 0u - v;
        if (m >= limit) m = limit;
        return 0u - m;
    }
    if (v >= limit) v = limit;
    return v;
}

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0041f613
void __cdecl FUN_0041f613(gp4::Regs* r) {
    const uint8_t* const owner = (const uint8_t*)r->esi;
    Unknown_006380d8* const rec = GP4_ARRAY(Unknown_006380d8, 0x006380d8);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00601f14);
    const double scale = GP4_GLOBAL(double, 0x006f29ec);

    uint32_t ebp = r->ebp;
    for (int k = 0; k < 4; k++) {
        rec[k].Unknown038 = (uint32_t)fistpExt32(fchs(dot4(&rec[k].Unknown114[0])) * gp4::x87::ext(scale));
        GP4_GLOBAL(int32_t, 0x006f2bf4) = fistpExt32(dot4(&rec[k].Unknown114[8]) * gp4::x87::ext(scale));
        ebp = clampMagnitude(GP4_GLOBAL(uint32_t, 0x006f2bf4), limit);
        rec[k].Unknown0c8 = ebp;
    }

    const int64_t p01 = (int64_t)(int32_t)((uint32_t)rec[0].Unknown114[0] - (uint32_t)rec[1].Unknown114[0])
                      * (int64_t)*(const int32_t*)(owner + 0x40c);
    const uint32_t d01 = (uint32_t)((uint64_t)p01 >> 10);
    rec[0].Unknown038 -= d01;
    rec[1].Unknown038 += d01;

    const int64_t p23 = (int64_t)(int32_t)((uint32_t)rec[2].Unknown114[0] - (uint32_t)rec[3].Unknown114[0])
                      * (int64_t)*(const int32_t*)(owner + 0x410);
    const uint32_t d23 = (uint32_t)((uint64_t)p23 >> 10);
    rec[2].Unknown038 -= d23;
    rec[3].Unknown038 += d23;

    rec[0].Unknown038 = clampMagnitude(rec[0].Unknown038, limit);
    rec[1].Unknown038 = clampMagnitude(rec[1].Unknown038, limit);
    rec[2].Unknown038 = clampMagnitude(rec[2].Unknown038, limit);
    const uint32_t x = rec[3].Unknown038;
    const uint32_t result = clampMagnitude(x, limit);
    rec[3].Unknown038 = result;

    // Flags of the last flag-setting instruction: the final NEG on the negative
    // path, the CMP against the limit on the non-negative path.
    uint32_t flags;
    if ((int32_t)x < 0) {
        const uint32_t m = 0u - result;
        flags = (m != 0u ? 1u : 0u)
            | parityFlag(result)
            | ((m ^ result) & 0x10u)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x80000000u) ? 0x80u : 0u)
            | (m == 0x80000000u ? 0x800u : 0u);
    } else {
        const uint32_t diff = x - limit;
        flags = (x < limit ? 1u : 0u)
            | parityFlag(diff)
            | ((x ^ limit ^ diff) & 0x10u)
            | (diff == 0u ? 0x40u : 0u)
            | ((diff & 0x80000000u) ? 0x80u : 0u)
            | ((((x ^ limit) & (x ^ diff)) & 0x80000000u) ? 0x800u : 0u);
    }

    r->eax = result;
    r->ebx = 0x006380d8u;
    r->edx = (uint32_t)((uint64_t)p23 >> 32);
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041f613, FUN_0041f613, "regs(esi:ptr[0x418]:layout=rec_0041f613) -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0041f613")
