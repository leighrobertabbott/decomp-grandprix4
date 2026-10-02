#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// imul + shrd eax, edx, shift: low 32 bits of the signed 64-bit product >> shift.
static inline int32_t mulShift(int32_t a, int32_t b, int shift) {
    const int64_t product = (int64_t)a * (int64_t)b;
    return (int32_t)(uint32_t)((uint64_t)product >> shift);
}

// cdq; shld edx, eax, 14; shl eax, 14; idiv d  for 0 <= |a| < |d| (no overflow),
// a and d of the same sign: the quotient is floor(|a| * 2^14 / |d|).
static inline int32_t divShift14(uint32_t absA, uint32_t absD) {
    uint32_t rem = absA;
    uint32_t q = 0;
    for (int i = 0; i < 14; i++) {
        rem <<= 1;
        q <<= 1;
        if (rem >= absD) {
            rem -= absD;
            q |= 1u;
        }
    }
    return (int32_t)q;
}

// FUNCTION: GP4 0x0043aac6
void __cdecl FUN_0043aac6(gp4::Regs* r) {
    int32_t* target = (int32_t*)r->ebx;
    const int32_t* origin = (const int32_t*)r->esi;

    int32_t& dx0 = GP4_GLOBAL(int32_t, 0x006079ec);
    int32_t& dy0 = GP4_GLOBAL(int32_t, 0x006079f0);
    int32_t& dz0 = GP4_GLOBAL(int32_t, 0x006079f4);
    int32_t& dx1 = GP4_GLOBAL(int32_t, 0x006079f8);
    int32_t& dy1 = GP4_GLOBAL(int32_t, 0x006079fc);
    int32_t& dz1 = GP4_GLOBAL(int32_t, 0x00607a00);

    dx0 = (int32_t)((uint32_t)target[0] - (uint32_t)origin[0]);
    dy0 = (int32_t)((uint32_t)target[1] - (uint32_t)origin[1]);
    dz0 = (int32_t)((uint32_t)target[2] - (uint32_t)origin[2]);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00603854);
    dx1 = (int32_t)((uint32_t)mulShift((int32_t)((uint32_t)target[6] - (uint32_t)origin[6]), scale, 16) + (uint32_t)dx0);
    dy1 = (int32_t)((uint32_t)mulShift((int32_t)((uint32_t)target[7] - (uint32_t)origin[7]), scale, 16) + (uint32_t)dy0);
    dz1 = (int32_t)((uint32_t)mulShift((int32_t)((uint32_t)target[8] - (uint32_t)origin[8]), scale, 16) + (uint32_t)dz0);

    const uint32_t dot01 = (uint32_t)mulShift(dx0, dx1, 14) + (uint32_t)mulShift(dy0, dy1, 14)
                         + (uint32_t)mulShift(dz0, dz1, 14);
    const uint32_t len0 = (uint32_t)mulShift(dx0, dx0, 14) + (uint32_t)mulShift(dy0, dy0, 14)
                        + (uint32_t)mulShift(dz0, dz0, 14);
    const uint32_t len1 = (uint32_t)mulShift(dx1, dx1, 14) + (uint32_t)mulShift(dy1, dy1, 14)
                        + (uint32_t)mulShift(dz1, dz1, 14);

    uint32_t denom = len0 - dot01 * 2u + len1;
    if (denom == 0) denom = 1;
    const uint32_t numer = len0 - dot01;

    int32_t t;
    if (((numer ^ denom) & 0x80000000u) != 0) {
        t = 0;
    } else {
        const int32_t absN = ((int32_t)numer < 0) ? (int32_t)(0u - numer) : (int32_t)numer;
        const int32_t absD = ((int32_t)denom < 0) ? (int32_t)(0u - denom) : (int32_t)denom;
        if (absN < absD) {
            t = divShift14((uint32_t)absN, (uint32_t)absD);
        } else {
            t = 0x4000;
        }
    }

    target[0] = (int32_t)((uint32_t)mulShift((int32_t)((uint32_t)dx1 - (uint32_t)dx0), t, 14)
                          + (uint32_t)dx0 + (uint32_t)origin[0]);
    target[1] = (int32_t)((uint32_t)mulShift((int32_t)((uint32_t)dy1 - (uint32_t)dy0), t, 14)
                          + (uint32_t)dy0 + (uint32_t)origin[1]);
    const uint32_t partial = (uint32_t)mulShift((int32_t)((uint32_t)dz1 - (uint32_t)dz0), t, 14)
                           + (uint32_t)dz0;
    const uint32_t addend = (uint32_t)origin[2];
    const uint32_t result = partial + addend;
    target[2] = (int32_t)result;

    // pushal/popal restore every GPR; the flags are those of the final add.
    const uint32_t flags = (result < partial ? 1u : 0u)
        | parityFlag(result)
        | ((partial ^ addend ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) ? 0x80u : 0u)
        | (((~(partial ^ addend) & (partial ^ result)) & 0x80000000u) ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043aac6, FUN_0043aac6, "regs(ebx:ptr[0x2c]:bytes, esi:ptr[0x2c]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
