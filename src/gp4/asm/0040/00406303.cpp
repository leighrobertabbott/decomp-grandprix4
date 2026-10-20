#include <gp4/gp4.h>
#include <gp4/x87.h>

// Record layout touched through edi / ebx (only the fields this routine reads).
struct Unknown_00406303 {
    uint8_t Unknown000[0xd4];
    int16_t UnknownD4;
    int16_t UnknownD6;
    uint8_t UnknownD8[0x04];
    int16_t UnknownDC;
    int16_t UnknownDE;
    uint8_t UnknownE0[0x0c];
    int32_t UnknownEC;
    int32_t UnknownF0;
    uint8_t UnknownF4[0x04];
    int32_t UnknownF8;
    uint8_t UnknownFC[0x2c];
    int16_t Unknown128;
    int16_t Unknown12a;
};

static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline bool negative(uint32_t v) { return (v & 0x80000000u) != 0u; }

// Angle (scaled atan2) from the reference point at 0x6278c0/0x6278c8 to a point
// given as (16-bit coarse << 8) + 32-bit fine coordinates.
static inline int32_t angleTo(int16_t coarseY, int32_t fineY, int16_t coarseX, int32_t fineX) {
    const int32_t dy = (int32_t)(GP4_GLOBAL(uint32_t, 0x006278c0)
        - (((uint32_t)(int32_t)coarseY << 8) + (uint32_t)fineY));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = dy;
    const int32_t dx = (int32_t)(GP4_GLOBAL(uint32_t, 0x006278c8)
        - (((uint32_t)(int32_t)coarseX << 8) + (uint32_t)fineX));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = dx;
    const gp4::x87::Ext angle = gp4::x87::fpatan(gp4::x87::ext_i32(dy), gp4::x87::ext_i32(dx))
        * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2be4));
    const int32_t result = gp4::x87::fistp32(angle);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = result;
    return result;
}

// Is angle a within the circular sector bounded by p and q (sign-of-difference
// tests in the original order)? *flags receives the flags of the last subtraction.
static inline bool withinSector(uint32_t a, uint32_t p, uint32_t q, uint32_t* flags) {
    if (!negative(p - q)) {
        *flags = subFlags32(a, q);
        if (negative(a - q)) return false;
        *flags = subFlags32(a, p);
        return negative(a - p);
    }
    *flags = subFlags32(a, q);
    if (!negative(a - q)) return true;
    *flags = subFlags32(a, p);
    return negative(a - p);
}

// FUNCTION: GP4 0x00406303
void __cdecl FUN_00406303(gp4::Regs* r) {
    const Unknown_00406303* self = (const Unknown_00406303*)r->edi;
    const Unknown_00406303* next = (const Unknown_00406303*)r->ebx;
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x006278ac);
    uint32_t flags = 0u;
    bool inside = false;

    const int32_t a1 = angleTo(self->UnknownD4, self->UnknownF0, self->UnknownDC, self->UnknownF8);
    GP4_GLOBAL(int32_t, 0x006278f8) = a1;
    const uint32_t c1 = (uint32_t)self->UnknownEC + 0x40000000u + offset;
    const uint32_t d1 = (uint32_t)(int32_t)self->Unknown128 << 16;
    if (withinSector((uint32_t)a1, c1, d1, &flags)) {
        const int32_t a2 = angleTo(self->UnknownD6, self->UnknownF0, self->UnknownDE, self->UnknownF8);
        GP4_GLOBAL(int32_t, 0x006278fc) = a2;
        const uint32_t c2 = (uint32_t)self->UnknownEC - 0x40000000u - offset;
        const uint32_t d2 = (uint32_t)(int32_t)self->Unknown12a << 16;
        if (withinSector((uint32_t)a2, d2, c2, &flags)) {
            const int32_t a3 = angleTo(next->UnknownD4, next->UnknownF0, next->UnknownDC, next->UnknownF8);
            GP4_GLOBAL(int32_t, 0x006278f0) = a3;
            const uint32_t d3 = ((uint32_t)(int32_t)self->Unknown128 << 16) + 0x80000000u;
            const uint32_t c3 = (uint32_t)next->UnknownEC + 0x40000000u - offset;
            if (withinSector((uint32_t)a3, d3, c3, &flags)) {
                const int32_t a4 = angleTo(next->UnknownD6, next->UnknownF0, next->UnknownDE, next->UnknownF8);
                GP4_GLOBAL(int32_t, 0x006278f4) = a4;
                const uint32_t d4 = ((uint32_t)(int32_t)self->Unknown12a << 16) + 0x80000000u;
                const uint32_t c4 = (uint32_t)next->UnknownEC - 0x40000000u + offset;
                inside = withinSector((uint32_t)a4, c4, d4, &flags);
            }
        }
    }

    // clc / stc keep the other flags of the last subtraction
    flags = (flags & ~1u) | (inside ? 1u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00406303, FUN_00406303, "regs(edi:ptr[0x132]:bytes, ebx:ptr[0x100]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
