#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right` (cmp, and neg as 0 - value).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Q14 cosine of a 16-bit angle from the word table at 0x007c5c5c (byte offset (|a| >> 2) & 0xfffe).
static inline int32_t cosQ14(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const uint32_t offset = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
    return GP4_ARRAY(int16_t, 0x007c5c5c)[offset >> 1];
}

static inline int64_t mul64(uint32_t a, uint32_t b) {
    return static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
}

static inline uint32_t lowShift14(int64_t product) {
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

static inline uint32_t highDword(int64_t product) {
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
}

static inline void clearX87(gp4::Regs& c) {
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
}

// (x + i*y) * (c + i*s) through 0x00401d41 on 80-bit values, both parts fistp'd.
static inline void rotate(const gp4::Regs& base, const Ext& x, const Ext& y, const Ext& c, const Ext& s,
                          int32_t* real, int32_t* imag) {
    gp4::Regs product = base;
    clearX87(product);
    gp4::x87::set_in_st(product, 0, x);
    gp4::x87::set_in_st(product, 1, y);
    gp4::x87::set_in_st(product, 2, c);
    gp4::x87::set_in_st(product, 3, s);
    product.st_in = 4;
    product.st_out = 2;
    gp4::call_regs(0x00401d41, product);
    *real = gp4::x87::fistp32(gp4::x87::out_st(product, 0));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = *real;
    *imag = gp4::x87::fistp32(gp4::x87::out_st(product, 1));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = *imag;
}

// esi = a car record. The reference point 0x601ad4/0x601ad8 is the car position (+0/+4), moved by
// [0x601354] along the heading word +0xe when byte [0x601ba8] is set. The target point (0x601b18,
// 0x601b1c) is, for a car with +0x25f bit 1, its grid-slot point (0x0043207c / 0x00432095 record,
// lateral offset from that record's angle, then 0x00433c4c rotated twice through 0x00401d41);
// otherwise the look-ahead point: 0x004292a3 gives the distance, 0x00429339 / 0x004295fe the record
// and 14-bit fraction (0x601b24 / 0x601b20), 0x00429431, 0x00403938, 0x00403b65 the lateral
// position and 0x00404796 the world point. With the distance d (fsqrt, at least 1, in 0x601acc) and
// the angle a of the target relative to the heading (0x00401315; reversed for +0x25f bit 1 or for
// +0x71 bit 2 with word +0x9a >= 0x5e), r = max(|cos a| * d >> 14, 0x240) and the steering value
// is 0x00429843 (byte [0x601ba8] set) or 2a * [0x601334] / r, scaled by [0x601ae8] >> 14 and clamped
// to +-[0x6013ac]; it is stored in word +0xb4. ecx and edi are saved and restored.
// FUNCTION: GP4 0x004298e4
void __cdecl FUN_004298e4(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    clearX87(c);

    GP4_GLOBAL(uint32_t, 0x00601ad4) = GP4_FIELD(uint32_t, car, 0x0);
    c.eax = GP4_FIELD(uint32_t, car, 0x4);
    GP4_GLOBAL(uint32_t, 0x00601ad8) = c.eax;
    if (GP4_GLOBAL(uint8_t, 0x00601ba8) != 0u) {
        const uint16_t heading = GP4_FIELD(uint16_t, car, 0xe);
        int64_t product = mul64(static_cast<uint32_t>(cosQ14(static_cast<uint16_t>(0x4000u - heading))),
                                GP4_GLOBAL(uint32_t, 0x00601354));
        GP4_GLOBAL(uint32_t, 0x00601ad4) = lowShift14(product) + GP4_FIELD(uint32_t, car, 0x0);
        const int32_t cosine = cosQ14(heading);
        c.ecx = static_cast<uint16_t>(cosine);
        product = mul64(static_cast<uint32_t>(cosine), GP4_GLOBAL(uint32_t, 0x00601354));
        c.eax = lowShift14(product) + GP4_FIELD(uint32_t, car, 0x4);
        c.edx = highDword(product);
        GP4_GLOBAL(uint32_t, 0x00601ad8) = c.eax;
    }

    if ((GP4_FIELD(uint8_t, car, 0x25f) & 2u) != 0u) {
        gp4::call_regs(0x0043207c, c);
        gp4::call_regs(0x00432095, c);
        const uint32_t slot = c.eax;
        c.edi = slot;
        GP4_GLOBAL(uint32_t, 0x00601b24) = slot;
        uint16_t angle = static_cast<uint16_t>(
            static_cast<int32_t>(GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(slot)), 0xec)) >> 16);
        if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0) {
            angle = static_cast<uint16_t>(angle - 0x4000u);
        } else {
            angle = static_cast<uint16_t>(angle + 0x4000u);
        }
        angle = static_cast<uint16_t>(angle + GP4_GLOBAL(uint16_t, 0x0062d2b0) - GP4_FIELD(uint16_t, car, 0xe) + 0x8000u);
        if (static_cast<int16_t>(angle) < 0) {
            angle = static_cast<uint16_t>(0u - angle);
        }
        if (static_cast<int16_t>(angle) >= 0x4000) {
            angle = static_cast<uint16_t>(0x8000u - angle);
        }
        const int64_t offset = mul64(static_cast<uint32_t>(cosQ14(static_cast<uint16_t>(0x4000u - angle))),
                                     GP4_GLOBAL(uint32_t, 0x00601c08));
        c.ebp = lowShift14(offset) + GP4_GLOBAL(uint32_t, 0x00601c0c);
        c.eax = c.ebp;
        c.edx = highDword(offset);
        gp4::call_regs(0x00433c4c, c);

        const int32_t dx = static_cast<int32_t>(c.eax - GP4_FIELD(uint32_t, car, 0x0));
        const int32_t dy = static_cast<int32_t>(c.edx - GP4_FIELD(uint32_t, car, 0x4));
        GP4_GLOBAL(uint32_t, 0x006f2bf4) = c.ecx;
        const Ext turn = gp4::x87::ext_i32(static_cast<int32_t>(c.ecx)) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc));
        Ext sine;
        Ext cosine;
        gp4::x87::fsincos(turn, &sine, &cosine);

        int32_t alongX;
        int32_t alongY;
        rotate(c, gp4::x87::ext_i32(dx), gp4::x87::ext_i32(dy), cosine, sine, &alongX, &alongY);
        c.ebp <<= 8;
        const int32_t lateral = static_cast<int32_t>(0u - c.ebp);
        int32_t worldX;
        int32_t worldY;
        rotate(c, gp4::x87::ext_i32(alongX), gp4::x87::ext_i32(lateral), cosine, gp4::x87::fchs(sine), &worldX, &worldY);
        c.eax = static_cast<uint32_t>(worldX) + GP4_FIELD(uint32_t, car, 0x0);
        c.edx = static_cast<uint32_t>(worldY) + GP4_FIELD(uint32_t, car, 0x4);
    } else {
        gp4::call_regs(0x004292a3, c);
        if ((GP4_FIELD(uint8_t, car, 0x26b) & 4u) != 0u) {
            c.eax = 0u;
        }
        GP4_GLOBAL(uint32_t, 0x00601acc) = c.eax;
        const uint32_t reach = static_cast<uint32_t>(static_cast<int32_t>(c.eax) >> 4)
            + static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x66)));
        c.ecx = reach & 0x3fffu;
        GP4_GLOBAL(uint32_t, 0x00601b20) = c.ecx;
        c.edx = static_cast<uint32_t>(static_cast<int32_t>(reach) >> 14) * 0x160u;
        c.eax = GP4_FIELD(uint32_t, car, 0x1c);
        gp4::call_regs(0x00429339, c);
        c.edi = c.eax;
        GP4_GLOBAL(uint32_t, 0x00601b24) = c.edi;
        gp4::call_regs(0x004295fe, c);

        const uint32_t segment = GP4_GLOBAL(uint32_t, 0x00601b24);
        c.edi = segment;
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(segment));
        const int32_t start = GP4_FIELD(int16_t, record, 0x16);
        c.ebp = static_cast<uint32_t>(start);
        const int64_t blend = mul64(static_cast<uint32_t>(GP4_FIELD(int16_t, record, 0x176) - start),
                                    GP4_GLOBAL(uint32_t, 0x00601b20));
        c.eax = lowShift14(blend) + static_cast<uint32_t>(start);
        c.edx = highDword(blend);
        GP4_GLOBAL(uint32_t, 0x00601b28) = c.eax;
        gp4::call_regs(0x00429431, c);

        const uint32_t savedFraction = GP4_GLOBAL(uint32_t, 0x00601b34);
        GP4_GLOBAL(uint32_t, 0x00601b34) = GP4_GLOBAL(uint32_t, 0x00601b20);
        c.eax = GP4_GLOBAL(uint32_t, 0x00601b28);
        c.edx = GP4_FIELD(uint8_t, car, 0x1cf);
        gp4::call_regs(0x00403938, c);
        c.ebp = GP4_FIELD(uint32_t, car, 0x290);
        c.edx = (GP4_FIELD(uint8_t, car, 0x1ce) & 0x80u) != 0u ? 0xffffffffu : 0u;
        gp4::call_regs(0x00403b65, c);
        GP4_GLOBAL(uint32_t, 0x00601b34) = savedFraction;
        c.edx = GP4_GLOBAL(uint32_t, 0x00601b20);
        gp4::call_regs(0x00404796, c);
    }

    GP4_GLOBAL(uint32_t, 0x00601b18) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00601b1c) = c.edx;
    const int32_t toX = static_cast<int32_t>(c.eax - GP4_GLOBAL(uint32_t, 0x00601ad4));
    const int32_t toY = static_cast<int32_t>(c.edx - GP4_GLOBAL(uint32_t, 0x00601ad8));
    const Ext ex = gp4::x87::ext_i32(toX);
    const Ext ey = gp4::x87::ext_i32(toY);
    const int32_t rounded = gp4::x87::fistp32(gp4::x87::fsqrt(ex * ex + ey * ey));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded;
    const uint32_t distance = rounded != 0 ? static_cast<uint32_t>(rounded) : 1u;
    GP4_GLOBAL(uint32_t, 0x00601acc) = distance;

    c.eax = static_cast<uint32_t>(toX);
    c.ecx = distance;
    c.edx = static_cast<uint32_t>(toY);
    gp4::call_regs(0x00401315, c);
    uint16_t relative = static_cast<uint16_t>(c.eax - GP4_FIELD(uint16_t, car, 0xe));
    if ((GP4_FIELD(uint8_t, car, 0x25f) & 2u) != 0u
        || ((GP4_FIELD(uint8_t, car, 0x71) & 4u) != 0u && GP4_FIELD(int16_t, car, 0x9a) >= 0x5e)) {
        relative = static_cast<uint16_t>(0u - static_cast<uint16_t>(relative + 0x8000u));
    }
    GP4_GLOBAL(int32_t, 0x00601b14) = static_cast<int16_t>(relative);
    int32_t factor = cosQ14(relative);
    if (factor < 0) {
        factor = -factor;
    }
    uint32_t radius = lowShift14(mul64(static_cast<uint32_t>(factor), distance));
    if (static_cast<int32_t>(radius) < 0x240) {
        radius = 0x240u;
    }
    c.ecx = radius;

    if (GP4_GLOBAL(uint8_t, 0x00601ba8) != 0u) {
        gp4::call_regs(0x00429843, c);
    } else {
        int32_t remainder;
        const int64_t numerator = mul64(GP4_GLOBAL(uint32_t, 0x00601b14) << 1, GP4_GLOBAL(uint32_t, 0x00601334));
        c.eax = static_cast<uint32_t>(gp4::idiv64(numerator, static_cast<int32_t>(radius), &remainder));
        c.edx = static_cast<uint32_t>(remainder);
    }
    const int64_t scaled = mul64(c.eax, GP4_GLOBAL(uint32_t, 0x00601ae8));
    uint32_t steer = lowShift14(scaled);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006013ac);
    uint32_t flags;
    if (static_cast<int32_t>(steer) < 0) {
        uint32_t magnitude = 0u - steer;
        if (magnitude >= limit) {
            magnitude = limit;
        }
        flags = subFlags32(0u, magnitude);
        steer = 0u - magnitude;
    } else {
        flags = subFlags32(steer, limit);
        if (steer >= limit) {
            steer = limit;
        }
    }
    GP4_FIELD(uint16_t, car, 0xb4) = static_cast<uint16_t>(steer);

    r->eax = steer;
    r->ebx = c.ebx;
    r->edx = highDword(scaled);
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004298e4, FUN_004298e4, "regs(esi:ptr[0x420]:layout=car_004129fa+car_00431ffb+car_0040362e+car_00426fa5+rec_004295fe+car_004035c3+car_004035f7) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00412af5+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+state_004059ef+geom_008943a0+rec_00433c4c+state_0042882e+state_00435945+state_004295fe+ref_004298e4")
