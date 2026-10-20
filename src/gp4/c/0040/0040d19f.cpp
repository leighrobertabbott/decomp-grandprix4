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

// Arithmetic flags of a 32-bit ADD a + b.
static inline uint32_t addFlags(uint32_t a, uint32_t b) {
    const uint32_t res = a + b;
    return (res < a ? 1u : 0u)
        | parityFlag(res)
        | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u)
        | ((res & 0x80000000u) ? 0x80u : 0u)
        | (((~(a ^ b) & (a ^ res)) & 0x80000000u) ? 0x800u : 0u);
}

// Low 32 bits of (a * b) >> 14 (imul + shrd eax, edx, 14).
static inline uint32_t mulShr14(uint32_t a, uint32_t b) {
    const int64_t p = (int64_t)(int32_t)a * (int64_t)(int32_t)b;
    return (uint32_t)((uint64_t)p >> 14);
}

// FUNCTION: GP4 0x0040d19f
// Walks the 0x160-byte records from edi up to ebx, projecting each record's point
// (0x0040d17c) onto the axis (0x0062780c, 0x00627810) about (0x006277f8, 0x00627800);
// stops at the first non-negative distance and interpolates between it and the
// previous record. Carry clear: no previous record. Callee-saved registers survive.
void __cdecl FUN_0040d19f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t end = r->ebx;
    uint32_t rec = r->edi;
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    GP4_GLOBAL(int32_t, 0x006277f4) = -1;

    for (;;) {
        c.eax = (eax & ~0xffu) | GP4_GLOBAL(uint8_t, 0x00627808);
        c.edi = rec;
        gp4::call_regs(0x0040d17c, c);
        GP4_GLOBAL(uint32_t, 0x006277c8) = c.eax;
        GP4_GLOBAL(uint32_t, 0x006277cc) = c.ecx;
        GP4_GLOBAL(uint32_t, 0x006277d0) = c.edx;
        GP4_GLOBAL(uint32_t, 0x006277dc) = rec;

        const uint32_t across = mulShr14(GP4_GLOBAL(uint32_t, 0x006277d0) - GP4_GLOBAL(uint32_t, 0x00627800),
                                         GP4_GLOBAL(uint32_t, 0x00627810));
        GP4_GLOBAL(uint32_t, 0x006277d4) =
            mulShr14(GP4_GLOBAL(uint32_t, 0x006277c8) - GP4_GLOBAL(uint32_t, 0x006277f8),
                     GP4_GLOBAL(uint32_t, 0x0062780c)) - across;
        const uint32_t along = mulShr14(GP4_GLOBAL(uint32_t, 0x006277c8) - GP4_GLOBAL(uint32_t, 0x006277f8),
                                        GP4_GLOBAL(uint32_t, 0x00627810));
        const int64_t p = (int64_t)(int32_t)(GP4_GLOBAL(uint32_t, 0x006277d0) - GP4_GLOBAL(uint32_t, 0x00627800))
                        * (int64_t)GP4_GLOBAL(int32_t, 0x0062780c);
        const uint32_t dist = (uint32_t)((uint64_t)p >> 14) + along;
        edx = (uint32_t)((uint64_t)p >> 32);
        GP4_GLOBAL(uint32_t, 0x006277d8) = dist;
        c.ebp = along;
        c.edx = edx;
        eax = dist;
        if ((int32_t)dist >= 0) break;

        GP4_GLOBAL(uint32_t, 0x006277e0) = GP4_GLOBAL(uint32_t, 0x006277c8);
        GP4_GLOBAL(uint32_t, 0x006277e4) = GP4_GLOBAL(uint32_t, 0x006277cc);
        GP4_GLOBAL(uint32_t, 0x006277e8) = GP4_GLOBAL(uint32_t, 0x006277d0);
        GP4_GLOBAL(uint32_t, 0x006277ec) = GP4_GLOBAL(uint32_t, 0x006277d4);
        GP4_GLOBAL(uint32_t, 0x006277f0) = GP4_GLOBAL(uint32_t, 0x006277d8);
        eax = GP4_GLOBAL(uint32_t, 0x006277dc);
        GP4_GLOBAL(uint32_t, 0x006277f4) = eax;
        rec += 0x160u;
        if (rec == end) break;
    }

    if (GP4_GLOBAL(int32_t, 0x006277f4) == -1) {
        r->eax = eax;
        r->edx = edx;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;  // cmp equal (ZF, PF), then clc
        return;
    }

    uint32_t span = GP4_GLOBAL(uint32_t, 0x006277d8) - GP4_GLOBAL(uint32_t, 0x006277f0);
    if (span == 0) span = 1;
    int32_t rem;
    const int32_t t = gp4::idiv64((int64_t)(int32_t)(0u - GP4_GLOBAL(uint32_t, 0x006277f0)) << 14,
                                  (int32_t)span, &rem);
    const uint32_t side = mulShr14(GP4_GLOBAL(uint32_t, 0x006277cc) - GP4_GLOBAL(uint32_t, 0x006277e4), (uint32_t)t)
                        + GP4_GLOBAL(uint32_t, 0x006277e4);
    uint32_t v = mulShr14(GP4_GLOBAL(uint32_t, 0x006277d4) - GP4_GLOBAL(uint32_t, 0x006277ec), (uint32_t)t)
               + GP4_GLOBAL(uint32_t, 0x006277ec);

    uint32_t outEax = 0;
    uint32_t outEdx = 0;
    uint32_t flags;
    uint32_t limit;
    int32_t width;
    uint8_t* car;
    bool zero = false;
    if ((int8_t)GP4_GLOBAL(uint8_t, 0x00868582) < 0) {
        flags = subFlags(v, 0xfffff000u);
        if ((int32_t)v > -0x1000) {
            zero = true;
        } else {
            car = GP4_GLOBAL(uint8_t*, 0x00627804);
            limit = GP4_FIELD(uint32_t, car, 0x100);
            width = (int32_t)GP4_FIELD(int16_t, car, 0x108) << 6;
        }
    } else {
        flags = subFlags(v, 0x1000u);
        if ((int32_t)v < 0x1000) {
            zero = true;
        } else {
            car = GP4_GLOBAL(uint8_t*, 0x00627804);
            limit = GP4_FIELD(uint32_t, car, 0x10c);
            width = (int32_t)GP4_FIELD(int16_t, car, 0x114) << 6;
        }
    }
    if (!zero) {
        if ((int32_t)v < 0) v = 0u - v;
        if ((int32_t)v > (int32_t)limit) v = limit;
        if (limit == 0) limit = 1;
        const int32_t q = gp4::idiv64((int64_t)((uint64_t)v << 14), (int32_t)limit, &rem);
        const int64_t pw = (int64_t)width * (int64_t)q;
        const uint32_t off = 0u - ((uint32_t)((uint64_t)pw >> 14) + GP4_GLOBAL(uint32_t, 0x006277fc));
        outEdx = off + side;
        flags = addFlags(off, side);
        outEax = (uint32_t)q;
    }
    r->eax = outEax;
    r->edx = outEdx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | 1u;  // stc
}
GP4_IMPL(0x0040d19f, FUN_0040d19f, "regs(eax:u32, edx:u32, ebx:rec[0x8943a0,0x160,11..19], edi:rec[0x8943a0,0x160,0..7]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0040d19f")
