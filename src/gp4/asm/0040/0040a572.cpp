#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit `sub left, right`.
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = (uint16_t)(left - right);
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) ? 0x800u : 0u);
}

// Flags of the 16-bit `add left, right`.
static inline uint32_t addFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = (uint16_t)(left + right);
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) ? 0x80u : 0u)
        | ((((left ^ result) & (right ^ result)) & 0x8000u) ? 0x800u : 0u);
}

static inline uint32_t withLow16(uint32_t reg, uint32_t low) {
    return (reg & 0xffff0000u) | (low & 0xffffu);
}

static inline gp4::Regs scratch(const gp4::Regs& s) {
    gp4::Regs t = s;
    t.st_in = 0;
    t.st_out = 0;
    t.st_xin_mask = 0;
    t.st_xout_mask = 0;
    return t;
}

// 0x00401000 inside pushal/popal: the signed 64-bit product of a (dx:ax) and b (cx:bx).
static inline uint64_t product64(const gp4::Regs& s, uint32_t a, uint32_t b) {
    gp4::Regs t = scratch(s);
    t.eax = withLow16(s.eax, a);
    t.edx = withLow16(s.edx, a >> 16);
    t.ebx = withLow16(s.ebx, b);
    t.ecx = withLow16(s.ecx, b >> 16);
    gp4::call_regs(0x00401000, t);
    return (uint64_t)(t.eax & 0xffffu) | ((uint64_t)(t.edx & 0xffffu) << 16)
        | ((uint64_t)(t.ebx & 0xffffu) << 32) | ((uint64_t)(t.ecx & 0xffffu) << 48);
}

// 0x004011e5 inside pushal/popal: the 32-bit table value (dx:ax) of a 16-bit angle.
static inline uint32_t tableValue(const gp4::Regs& s, uint16_t angle) {
    gp4::Regs t = scratch(s);
    t.eax = withLow16(s.eax, angle);
    gp4::call_regs(0x004011e5, t);
    return (t.eax & 0xffffu) | (t.edx << 16);
}

// edi = a 0x40aadb key-frame record (words +0/+4/+8/+0x14, byte +0x21 holds the fraction bits),
// esi = the car. Rotates the offset ([0x62ac6c], [0x62ac70]) - record position by the record
// angle (sine/cosine pair from 0x40a006 at 0x934de8 / 0x934dec, products by 0x401000, >> 30),
// then with [0x62ac64] == 0 stores the lateral word +0x90 from the car heading (+0x18) via
// 0x4011e5 / 0x409e0b, otherwise the lateral offset from the root of K^2 - y^2 (0x409cf2) and the
// heading +0x18 from the angle of the scaled pair (0x409f2a).
// FUNCTION: GP4 0x0040a572
void __cdecl FUN_0040a572(gp4::Regs* r) {
    uint8_t* const rec = (uint8_t*)r->edi;
    uint8_t* const car = (uint8_t*)r->esi;

    const uint8_t frac = GP4_FIELD(uint8_t, rec, 0x21);
    const uint32_t posX = ((uint32_t)(int32_t)GP4_FIELD(int16_t, rec, 4) << 3) | (frac & 7u);
    const uint32_t posY = ((uint32_t)(int32_t)GP4_FIELD(int16_t, rec, 8) << 3) | ((uint32_t)frac >> 4);
    const uint32_t x = GP4_GLOBAL(uint32_t, 0x0062ac6c) - posX;
    const uint32_t y = GP4_GLOBAL(uint32_t, 0x0062ac70) - posY;
    GP4_GLOBAL(uint32_t, 0x00934df0) = x;
    GP4_GLOBAL(uint32_t, 0x00934df4) = y;

    // imul word by [0x601240], then shl ax / rcl dx: bits 15..30 of the product.
    const int32_t tilt = (int32_t)(int16_t)(GP4_FIELD(int16_t, rec, 0x14) >> 1)
                       * (int32_t)GP4_GLOBAL(int16_t, 0x00601240);
    const uint16_t tiltWord = (uint16_t)((uint32_t)tilt >> 15);
    const uint16_t angle = (uint16_t)(GP4_FIELD(uint16_t, rec, 0) - tiltWord);
    GP4_GLOBAL(uint16_t, 0x0062a964) = angle;
    GP4_GLOBAL(uint16_t, 0x00934de8) = angle;

    gp4::Regs s = scratch(*r);
    s.eax = withLow16(posY, angle);
    s.edx = withLow16(r->edx, tiltWord);
    gp4::call_regs(0x0040a006, s);

    const uint32_t sine = GP4_GLOBAL(uint32_t, 0x00934de8);
    const uint32_t cosine = GP4_GLOBAL(uint32_t, 0x00934dec);
    GP4_GLOBAL(uint32_t, 0x00934de0) = GP4_GLOBAL(uint32_t, 0x00934df0);
    GP4_GLOBAL(uint32_t, 0x00934de4) = GP4_GLOBAL(uint32_t, 0x00934df4);

    const uint64_t sx = product64(s, sine, x);
    GP4_GLOBAL(uint32_t, 0x0062ad54) = (uint32_t)(sx >> 32);
    GP4_GLOBAL(uint32_t, 0x00934df0) = (uint32_t)sx;
    const uint64_t cy = product64(s, cosine, y);
    GP4_GLOBAL(uint32_t, 0x0062ad48) = (uint32_t)(cy >> 32);
    GP4_GLOBAL(uint32_t, 0x00934de4) = (uint32_t)cy;
    const uint64_t cx = product64(s, cosine, x);
    GP4_GLOBAL(uint32_t, 0x0062ad44) = (uint32_t)(cx >> 32);
    GP4_GLOBAL(uint32_t, 0x00934de0) = (uint32_t)cx;
    const uint64_t sy = product64(s, sine, y);
    GP4_GLOBAL(uint32_t, 0x0062ad58) = (uint32_t)(sy >> 32);
    GP4_GLOBAL(uint32_t, 0x00934df4) = (uint32_t)sy;

    // 64-bit sub/sbb and add/adc chains, then two 48-bit shl/rcl steps: bits 30..61.
    const uint32_t along = (uint32_t)((sx - cy) >> 30);
    const uint32_t across = (uint32_t)((sy + cx) >> 30);
    GP4_GLOBAL(uint32_t, 0x0062ad54) = along;
    GP4_GLOBAL(uint32_t, 0x00934df0) = along;
    GP4_GLOBAL(uint32_t, 0x0062ad58) = across;
    GP4_GLOBAL(uint32_t, 0x00934df4) = across;

    if ((GP4_GLOBAL(uint16_t, 0x0062ac64) | GP4_GLOBAL(uint16_t, 0x0062ac66)) == 0) {
        const uint16_t rel = (uint16_t)(GP4_FIELD(uint16_t, car, 0x18) - GP4_GLOBAL(uint16_t, 0x0062a964));
        GP4_GLOBAL(uint16_t, 0x00934de4) = rel;
        GP4_GLOBAL(uint16_t, 0x00934de0) = (uint16_t)(0x4000u - rel);
        GP4_GLOBAL(uint32_t, 0x00934de0) = tableValue(s, GP4_GLOBAL(uint16_t, 0x00934de0));
        GP4_GLOBAL(uint32_t, 0x00934de4) = tableValue(s, GP4_GLOBAL(uint16_t, 0x00934de4));

        const uint64_t p = product64(s, GP4_GLOBAL(uint32_t, 0x00934de0), GP4_GLOBAL(uint32_t, 0x00934df4));
        GP4_GLOBAL(uint32_t, 0x00934dec) = (uint32_t)(p >> 32);
        GP4_GLOBAL(uint32_t, 0x00934df4) = (uint32_t)p;

        // Signed 64/32 divide (0x409e0b): dividend cx:bx:dx:ax, divisor si:di.
        gp4::Regs d = scratch(s);
        d.ebx = withLow16(s.ebx, GP4_GLOBAL(uint16_t, 0x00934dec));
        d.ecx = withLow16(s.ecx, GP4_GLOBAL(uint16_t, 0x00934dee));
        d.eax = withLow16(s.eax, GP4_GLOBAL(uint16_t, 0x00934df4));
        d.edx = withLow16(s.edx, GP4_GLOBAL(uint16_t, 0x00934df6));
        d.edi = withLow16(s.edi, GP4_GLOBAL(uint16_t, 0x00934de4));
        d.esi = withLow16(s.esi, GP4_GLOBAL(uint16_t, 0x00934de6));
        gp4::call_regs(0x00409e0b, d);
        GP4_GLOBAL(uint16_t, 0x00934df4) = (uint16_t)d.eax;
        GP4_GLOBAL(uint16_t, 0x00934df6) = (uint16_t)d.edx;

        const uint16_t left = GP4_GLOBAL(uint16_t, 0x00934df0);
        const uint16_t right = GP4_GLOBAL(uint16_t, 0x00934df4);
        const uint16_t lateral = (uint16_t)(left - right);
        GP4_FIELD(uint16_t, car, 0x90) = lateral;

        r->eax = withLow16(s.eax, lateral);
        r->edx = withLow16(s.edx, (uint32_t)(cx >> 48));
        r->eflags = (r->eflags & ~0x8d5u) | subFlags16(left, right);
        return;
    }

    const uint32_t saved = GP4_GLOBAL(uint32_t, 0x00934df4);
    const uint64_t yy = product64(s, saved, saved);
    GP4_GLOBAL(uint32_t, 0x00934dec) = (uint32_t)(yy >> 32);
    GP4_GLOBAL(uint32_t, 0x00934df4) = (uint32_t)yy;
    const uint32_t k = GP4_GLOBAL(uint32_t, 0x0062ac64);
    const uint64_t kk = product64(s, k, k);
    const uint64_t rest = kk - yy;
    GP4_GLOBAL(uint32_t, 0x00934de0) = (uint32_t)rest;
    GP4_GLOBAL(uint32_t, 0x00934de4) = (uint32_t)(rest >> 32);

    // Root of the 64-bit value cx:bx:dx:ax (0x409cf2), signed like K.
    gp4::Regs q = scratch(s);
    q.ebx = withLow16(s.ebx, GP4_GLOBAL(uint16_t, 0x00934de4));
    q.ecx = withLow16(s.ecx, GP4_GLOBAL(uint16_t, 0x00934de6));
    q.eax = withLow16(s.eax, GP4_GLOBAL(uint16_t, 0x00934de0));
    q.edx = withLow16(s.edx, GP4_GLOBAL(uint16_t, 0x00934de2));
    gp4::call_regs(0x00409cf2, q);
    uint32_t root = (q.eax & 0xffffu) | (q.edx << 16);
    if (GP4_GLOBAL(int16_t, 0x0062ac66) < 0) root = 0u - root;
    GP4_GLOBAL(uint32_t, 0x00934de0) = root;

    const uint32_t lateral = GP4_GLOBAL(uint32_t, 0x00934df0) - root;
    GP4_GLOBAL(uint32_t, 0x00934df0) = lateral;
    GP4_FIELD(uint16_t, car, 0x90) = (uint16_t)lateral;
    GP4_GLOBAL(uint32_t, 0x00934de4) = saved;

    // |K| (16-bit not/neg/inc pieces = 32-bit negation), halving both terms while |K| >= 0x7f00.
    uint32_t mag = GP4_GLOBAL(int16_t, 0x0062ac66) < 0 ? 0u - k : k;
    while ((int32_t)mag >= 0x7f00) {
        GP4_GLOBAL(int32_t, 0x00934de0) = GP4_GLOBAL(int32_t, 0x00934de0) >> 1;
        GP4_GLOBAL(int32_t, 0x00934de4) = GP4_GLOBAL(int32_t, 0x00934de4) >> 1;
        mag >>= 1;
    }

    gp4::Regs a = scratch(s);
    a.eax = withLow16(mag, GP4_GLOBAL(uint16_t, 0x00934de0));
    a.edx = withLow16(s.edx, GP4_GLOBAL(uint16_t, 0x00934de4));
    gp4::call_regs(0x00409f2a, a);
    uint16_t heading = (uint16_t)a.eax;
    if (GP4_GLOBAL(int16_t, 0x0062ac66) < 0) heading = (uint16_t)(heading + 0x4000u);
    else heading = (uint16_t)(heading - 0x4000u);
    const uint16_t base = GP4_GLOBAL(uint16_t, 0x0062a964);
    const uint16_t result = (uint16_t)(heading + base);
    GP4_FIELD(uint16_t, car, 0x18) = result;

    r->eax = withLow16(a.eax, result);
    r->edx = a.edx;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags16(heading, base);
}
GP4_IMPL(0x0040a572, FUN_0040a572, "regs(edi:ptr[0x28]:bytes, esi:ptr[0x98]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
