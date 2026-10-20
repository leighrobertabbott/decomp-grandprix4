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

// Flags of `cmp left, right` on 32 bits.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) >> 20) & 0x800u);
}

// Flags of `cmp left, right` / `sub left, right` on 16 bits.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | ((((left ^ right) & (left ^ result)) << 4) & 0x800u);
}

// Flags of `cmp left, right` on 8 bits.
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) << 4) & 0x800u);
}

// Flags of a 32-bit logic result (or / test): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value >> 24) & 0x80u);
}

// Flags of an 8-bit `test`: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// Low 32 bits of the 64-bit product a * b shifted right (imul; shrd eax, edx, n).
static inline uint32_t mulShift(int32_t a, int32_t b, int n) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> n);
}

static inline void clearX87(gp4::Regs& c) {
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
}

// Registers at the routine's `pop ebx; ret`: ebx, esi and edi keep their entry values.
static inline void leave(gp4::Regs* r, const gp4::Regs& c, uint32_t flags) {
    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// 0x00401d41: (a + i*b) * (c + i*d) on the x87 stack, all extended precision.
static void complexProduct(gp4::Regs& c, const Ext& a, const Ext& b, const Ext& cosine, const Ext& sine,
                           Ext* real, Ext* imag) {
    clearX87(c);
    gp4::x87::set_in_st(c, 0, a);
    gp4::x87::set_in_st(c, 1, b);
    gp4::x87::set_in_st(c, 2, cosine);
    gp4::x87::set_in_st(c, 3, sine);
    c.st_in = 4;
    c.st_out = 2;
    gp4::call_regs(0x00401d41, c);
    *real = gp4::x87::out_st(c, 0);
    *imag = gp4::x87::out_st(c, 1);
    clearX87(c);
}

// 0x00433c39: unless bit 0 of +0xe5 is set, the word +0xe is copied to +0x18 (through ax).
static void finish(gp4::Regs* r, gp4::Regs& c, uint8_t* car) {
    const uint32_t bit = GP4_FIELD(uint8_t, car, 0xe5) & 1u;
    if (bit == 0u) {
        const uint16_t heading = GP4_FIELD(uint16_t, car, 0xe);
        c.eax = (c.eax & 0xffff0000u) | heading;
        GP4_FIELD(uint16_t, car, 0x18) = heading;
    }
    leave(r, c, logicFlags8(bit));
}

// 0x004339ad: eax/ecx/edx hold the target point and angle of the callee; rotate the car's
// offsets and velocity into that frame, damp them and rotate back.
static void rotate(gp4::Regs* r, gp4::Regs& c, uint8_t* car) {
    GP4_GLOBAL(uint32_t, 0x00601bcc) = c.ecx;
    c.eax = GP4_FIELD(uint32_t, car, 0x0) - c.eax;
    GP4_GLOBAL(uint32_t, 0x00623e30) = c.eax;
    c.edx = GP4_FIELD(uint32_t, car, 0x4) - c.edx;
    GP4_GLOBAL(uint32_t, 0x00623e38) = c.edx;
    c.ecx = GP4_FIELD(uint32_t, car, 0xc) - c.ecx;
    const bool sideMode = GP4_GLOBAL(uint8_t, 0x00601bad) != 0u;
    if (sideMode && GP4_GLOBAL(uint8_t, 0x00601baf) != 0u) {
        c.ecx += 0x80000000u;
    }

    const Ext scale = gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc));
    Ext sine;
    Ext cosine;
    gp4::x87::fsincos(gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00601bcc)) * scale, &sine, &cosine);

    GP4_GLOBAL(int32_t, 0x006f2bf4) = GP4_FIELD(int32_t, car, 0xa8) >> 8;
    const Ext partB = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = GP4_FIELD(int32_t, car, 0xa4) >> 8;
    const Ext partA = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    Ext real;
    Ext imag;
    complexProduct(c, partA, partB, cosine, sine, &real, &imag);
    GP4_GLOBAL(int32_t, 0x00601bc4) = gp4::x87::fistp32(real);
    GP4_GLOBAL(int32_t, 0x00601bc8) = gp4::x87::fistp32(imag);

    const Ext offY = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00623e38));
    const Ext offX = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00623e30));
    complexProduct(c, offX, offY, cosine, sine, &real, &imag);
    GP4_GLOBAL(int32_t, 0x00601bbc) = gp4::x87::fistp32(real);
    GP4_GLOBAL(int32_t, 0x00601bc0) = gp4::x87::fistp32(imag);

    if (sideMode) {
        c.eax = GP4_GLOBAL(uint32_t, 0x00601bfc);
    } else {
        gp4::call_regs(0x00433590, c);
    }
    if (static_cast<int32_t>(c.eax) > GP4_GLOBAL(int32_t, 0x00601be4)) {
        c.eax = GP4_GLOBAL(uint32_t, 0x00601be4);
    }
    GP4_GLOBAL(uint32_t, 0x00601bd0) = c.eax;
    int32_t remainder;
    int32_t steps = gp4::idiv64(static_cast<int64_t>(static_cast<uint64_t>(c.eax)),
                                GP4_GLOBAL(int32_t, 0x00603854), &remainder);
    c.edx = static_cast<uint32_t>(remainder);
    if (steps < GP4_GLOBAL(int32_t, 0x00601bd8)) {
        steps = GP4_GLOBAL(int32_t, 0x00601bd8);
    }
    c.eax = static_cast<uint32_t>(steps);
    GP4_GLOBAL(int32_t, 0x00601bd4) = steps;

    gp4::call_regs(0x00433620, c);
    c.ebp = c.edx;
    const uint32_t along = c.eax;

    bool adjust = true;
    if (sideMode) {
        uint32_t top = static_cast<uint32_t>(static_cast<int32_t>(c.ecx) >> 24);
        if (GP4_GLOBAL(uint8_t, 0x00601bae) != 0u) {
            top = 0u - top;
        }
        if (((static_cast<uint8_t>(top) ^ GP4_GLOBAL(uint8_t, 0x00868582)) & 0x80u) != 0u) {
            adjust = false;
        }
    }
    if (adjust) {
        const int32_t q = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(c.ecx)),
                                      GP4_GLOBAL(int32_t, 0x00601bd4), &remainder);
        GP4_FIELD(int32_t, car, 0xc) -= q;
    }

    uint32_t numerator = along;
    if (sideMode) {
        const uint8_t top = static_cast<uint8_t>(GP4_GLOBAL(uint32_t, 0x00601bbc) >> 24);
        if (((top ^ GP4_GLOBAL(uint8_t, 0x00868582)) & 0x80u) != 0u) {
            numerator = 0u;
        }
    }
    const int32_t first = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(numerator)),
                                      GP4_GLOBAL(int32_t, 0x00601bd4), &remainder);

    bool divide;
    if (GP4_GLOBAL(uint8_t, 0x00601bac) != 0u) {
        if ((GP4_FIELD(uint8_t, car, 0x26b) & 8u) == 0u) {
            divide = false;
        } else if (GP4_GLOBAL(int32_t, 0x00601bc0) < 0) {
            divide = true;
        } else {
            divide = (GP4_FIELD(uint8_t, car, 0x25f) & 2u) == 0u;
        }
    } else if (sideMode) {
        divide = false;
    } else if (GP4_GLOBAL(int32_t, 0x00601bc0) >= 0) {
        divide = true;
    } else {
        divide = GP4_GLOBAL(int32_t, 0x00601bc8) < GP4_GLOBAL(int32_t, 0x00601bb0);
    }
    int32_t second = 0;
    if (divide) {
        second = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601bc0)),
                             GP4_GLOBAL(int32_t, 0x00601bd4), &remainder);
    }

    Ext sine2;
    Ext cosine2;
    gp4::x87::fsincos(gp4::x87::fchs(gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00601bcc))) * scale, &sine2, &cosine2);

    if (sideMode) {
        const uint8_t top = static_cast<uint8_t>(GP4_GLOBAL(uint32_t, 0x00601bc4) >> 24);
        if (((top ^ GP4_GLOBAL(uint8_t, 0x00868582)) & 0x80u) == 0u) {
            GP4_GLOBAL(uint32_t, 0x00601bc4) = mulShift(GP4_GLOBAL(int32_t, 0x00601bc4), GP4_GLOBAL(int32_t, 0x00601bf4), 14);
            const Ext velB = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00601bc8));
            const Ext velA = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00601bc4));
            complexProduct(c, velA, velB, cosine2, sine2, &real, &imag);
            const int32_t newA = gp4::x87::fistp32(real);
            GP4_GLOBAL(int32_t, 0x006f2bf4) = newA;
            GP4_FIELD(uint32_t, car, 0xa4) = static_cast<uint32_t>(newA) << 8;
            const int32_t newB = gp4::x87::fistp32(imag);
            GP4_GLOBAL(int32_t, 0x006f2bf4) = newB;
            GP4_FIELD(uint32_t, car, 0xa8) = static_cast<uint32_t>(newB) << 8;

            uint32_t spin = GP4_FIELD(uint32_t, car, 0xac);
            if (GP4_GLOBAL(uint8_t, 0x00601bae) != 0u) {
                spin = 0u - spin;
            }
            if (((static_cast<uint8_t>(spin >> 24) ^ GP4_GLOBAL(uint8_t, 0x00868582)) & 0x80u) == 0u) {
                GP4_FIELD(uint32_t, car, 0xac) = mulShift(GP4_FIELD(int32_t, car, 0xac), GP4_GLOBAL(int32_t, 0x00601bf0), 14);
            }
        }
    }

    GP4_GLOBAL(int32_t, 0x006f2bf4) = second;
    const Ext backY = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = first;
    const Ext backX = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    complexProduct(c, backX, backY, cosine2, sine2, &real, &imag);
    const int32_t dx = gp4::x87::fistp32(real);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = dx;
    const int32_t dy = gp4::x87::fistp32(imag);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = dy;
    c.eax = static_cast<uint32_t>(dx);
    c.edx = static_cast<uint32_t>(dy);
    GP4_FIELD(int32_t, car, 0x0) -= dx;
    GP4_FIELD(int32_t, car, 0x4) -= dy;
    finish(r, c, car);
}

// 0x0043399a: the target from 0x00433c4c (after 0x004048da unless bit 0 of +0xe5 is set).
static void fromLane(gp4::Regs* r, gp4::Regs& c, uint8_t* car) {
    if ((GP4_FIELD(uint8_t, car, 0xe5) & 1u) == 0u) {
        gp4::call_regs(0x004048da, c);
    }
    gp4::call_regs(0x00433c4c, c);
    rotate(r, c, car);
}

// 0x004337a0: steer the heading +0xc toward the target point of 0x00433c4c.
static void steer(gp4::Regs* r, gp4::Regs& c, uint8_t* car) {
    gp4::call_regs(0x00433c4c, c);
    c.eax = GP4_FIELD(uint32_t, car, 0x0) - c.eax;
    GP4_GLOBAL(uint32_t, 0x00623e30) = c.eax;
    c.edx = GP4_FIELD(uint32_t, car, 0x4) - c.edx;
    GP4_GLOBAL(uint32_t, 0x00623e38) = c.edx;
    gp4::call_regs(0x00401315, c);

    const int16_t difference = static_cast<int16_t>(GP4_FIELD(uint16_t, car, 0xe) - static_cast<uint16_t>(c.eax));
    c.edx = static_cast<uint32_t>(static_cast<int32_t>(difference));
    uint32_t magnitude = c.edx;
    if (static_cast<int32_t>(magnitude) < 0) {
        magnitude = 0u - magnitude;
    }
    c.eax = magnitude;
    const uint32_t dead = GP4_GLOBAL(uint32_t, 0x00601be8);
    if (static_cast<int32_t>(magnitude) < static_cast<int32_t>(dead)) {
        leave(r, c, subFlags32(magnitude, dead));
        return;
    }
    uint32_t excess = magnitude - dead;
    if (difference < 0) {
        excess = 0u - excess;
    }
    excess = 0u - excess;
    const uint32_t scaled = mulShift(static_cast<int32_t>(excess), GP4_GLOBAL(int32_t, 0x00601c00), 14);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(scaled)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    uint32_t turn = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16);
    c.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t cap = GP4_GLOBAL(uint32_t, 0x00601bec);
    if (static_cast<int32_t>(turn) < 0) {
        uint32_t m = 0u - turn;
        if (m >= cap) {
            m = cap;
        }
        turn = 0u - m;
    } else if (turn >= cap) {
        turn = cap;
    }
    c.eax = turn << 16;
    GP4_FIELD(uint32_t, car, 0xc) += c.eax;
    finish(r, c, car);
}

// FUNCTION: GP4 0x00433709
void __cdecl FUN_00433709(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t segment = r->edi;
    gp4::Regs c = *r;
    clearX87(c);

    const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);
    if (segment < pitStart) {
        leave(r, c, subFlags32(segment, pitStart));
        return;
    }
    {
        const uint32_t bit = GP4_FIELD(uint8_t, car, 0x7a) & 0x20u;
        if (bit != 0u) {
            leave(r, c, logicFlags8(bit));
            return;
        }
    }
    {
        const uint32_t bit = GP4_FIELD(uint8_t, car, 0x232) & 1u;
        if (bit == 0u) {
            leave(r, c, logicFlags8(bit));
            return;
        }
    }

    if ((GP4_FIELD(uint8_t, car, 0x26b) & 0x80u) == 0u) {
        // 0x0043390a
        const uint32_t low = GP4_GLOBAL(uint32_t, 0x00866f08) - 0xb00u;
        c.eax = low;
        if (segment < low) {
            leave(r, c, subFlags32(segment, low));
            return;
        }
        const uint32_t high = GP4_GLOBAL(uint32_t, 0x00866f0c) + 0xb00u;
        c.eax = high;
        if (segment >= high) {
            leave(r, c, subFlags32(segment, high));
            return;
        }
        GP4_GLOBAL(uint8_t, 0x00601bad) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x00601bac) = 0u;
        GP4_GLOBAL(uint32_t, 0x00601be4) = GP4_GLOBAL(uint32_t, 0x00601bdc);
        const int32_t lateral = GP4_FIELD(int16_t, car, 0x90);
        c.eax = static_cast<uint32_t>(lateral);
        int32_t reach;
        int32_t limit;
        if (GP4_GLOBAL(int8_t, 0x00868582) < 0) {
            if (lateral >= 0) {
                leave(r, c, logicFlags32(c.eax));
                return;
            }
            reach = 0 - lateral;
            limit = GP4_FIELD(int16_t, car, 0x11c);
        } else {
            if (lateral < 0) {
                leave(r, c, logicFlags32(c.eax));
                return;
            }
            reach = lateral;
            limit = GP4_FIELD(int16_t, car, 0x11e);
        }
        c.edx = static_cast<uint32_t>(limit) + 0x150u;
        c.eax = static_cast<uint32_t>(reach) + 0x7du;
        if (static_cast<int32_t>(c.eax) < static_cast<int32_t>(c.edx)) {
            leave(r, c, subFlags32(c.eax, c.edx));
            return;
        }
        gp4::call_regs(0x00433689, c);
        rotate(r, c, car);
        return;
    }

    const uint8_t kind = GP4_FIELD(uint8_t, car, 0xe4);
    if (static_cast<int8_t>(kind) < 1) {
        leave(r, c, subFlags8(kind, 1u));
        return;
    }
    if (static_cast<int8_t>(kind) > 5) {
        leave(r, c, subFlags8(kind, 5u));
        return;
    }

    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0x80u) {
        GP4_GLOBAL(uint8_t, 0x00601bad) = 0u;
        GP4_GLOBAL(uint8_t, 0x00601bac) = 0xffu;
        c.eax = GP4_GLOBAL(uint32_t, 0x00601bdc);
        GP4_GLOBAL(uint32_t, 0x00601be4) = c.eax;
        const uint8_t state = GP4_FIELD(uint8_t, car, 0x26b);
        if ((state & 2u) != 0u) {
            leave(r, c, logicFlags8(state & 2u));
            return;
        }
        if ((state & 8u) != 0u) {
            fromLane(r, c, car);
            return;
        }
        const uint32_t bit = GP4_FIELD(uint8_t, car, 0x25f) & 2u;
        if (bit == 0u) {
            leave(r, c, logicFlags8(bit));
            return;
        }
        steer(r, c, car);
        return;
    }

    // 0x00433832
    GP4_GLOBAL(uint8_t, 0x00601bad) = 0u;
    GP4_GLOBAL(uint8_t, 0x00601bac) = 0u;
    c.eax = GP4_GLOBAL(uint32_t, 0x00601be0);
    GP4_GLOBAL(uint32_t, 0x00601be4) = c.eax;
    if (kind != 1u) {
        fromLane(r, c, car);
        return;
    }
    {
        const uint32_t bit = GP4_FIELD(uint8_t, car, 0x230) & 0x80u;
        if (bit != 0u) {
            leave(r, c, logicFlags8(bit));
            return;
        }
    }
    {
        const uint16_t lane = GP4_FIELD(uint16_t, car, 0xea);
        if (lane != 0x20u) {
            leave(r, c, subFlags16(lane, 0x20u));
            return;
        }
    }
    gp4::call_regs(0x00432c21, c);
    const int32_t index = static_cast<int32_t>(c.eax);
    if (index < -1) {
        leave(r, c, subFlags32(c.eax, 0xffffffffu));
        return;
    }
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00601bb4);
    if (index > static_cast<int32_t>(limit)) {
        leave(r, c, subFlags32(c.eax, limit));
        return;
    }
    if (index >= static_cast<int32_t>(limit)) {
        c.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x66)));
        const uint32_t minimum = GP4_GLOBAL(uint32_t, 0x00601bb8);
        if (static_cast<int32_t>(c.eax) < static_cast<int32_t>(minimum)) {
            leave(r, c, subFlags32(c.eax, minimum));
            return;
        }
    }

    gp4::call_regs(0x0043b974, c);
    uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.ebx));
    if ((GP4_FIELD(uint8_t, other, 0x26b) & 0x80u) == 0u) {
        fromLane(r, c, car);
        return;
    }
    {
        const uint16_t lane = GP4_FIELD(uint16_t, other, 0xea);
        if (lane != 0x20u) {
            leave(r, c, subFlags16(lane, 0x20u));
            return;
        }
    }
    {
        const uint16_t ahead = GP4_FIELD(uint16_t, other, 0x68);
        const uint16_t mine = GP4_FIELD(uint16_t, car, 0x68);
        const uint16_t gap = static_cast<uint16_t>(ahead - mine);
        c.eax = (c.eax & 0xffff0000u) | gap;
        if ((gap & 0x8000u) != 0u) {
            leave(r, c, subFlags16(ahead, mine));
            return;
        }
    }
    {
        const uint32_t self = c.esi;
        c.esi = c.ebx;
        c.ebx = self;
        gp4::call_regs(0x00432c21, c);
        const uint32_t swapped = c.esi;
        c.esi = c.ebx;
        c.ebx = swapped;
    }
    if (c.eax == 0u) {
        leave(r, c, subFlags32(0u, 0u));
        return;
    }
    if (c.eax != 0xffffffffu) {
        leave(r, c, subFlags32(c.eax, 0xffffffffu));
        return;
    }
    {
        const uint16_t word = GP4_FIELD(uint16_t, other, 0x66);
        c.eax = (c.eax & 0xffff0000u) | word;
        if (static_cast<int16_t>(word) < 0x3000) {
            leave(r, c, subFlags16(word, 0x3000u));
            return;
        }
    }
    fromLane(r, c, car);
}
GP4_IMPL(0x00433709, FUN_00433709, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x420]:layout=car_00431ffb+rec_00432f18+car_004168fd+car_0042dd4b+car_00433709, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+search_0066d82c+list_0066fe3c+flags_00430449+state_00432330+state_00431ffb+state_004059ef+geom_008943a0+rec_00433c4c+rate_00603854+state_00433709+cars_00433709")
