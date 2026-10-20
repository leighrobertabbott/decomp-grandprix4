#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a TEST / logic op on an 8-bit result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a TEST / OR on a 32-bit result.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// Flags of the 32-bit "cmp lhs, rhs" / "sub lhs, rhs".
static inline uint32_t subFlags32(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = lhs - rhs;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit "cmp lhs, rhs".
static inline uint32_t subFlags16(uint32_t lhs, uint32_t rhs) {
    lhs &= 0xffffu;
    rhs &= 0xffffu;
    const uint32_t result = (lhs - rhs) & 0xffffu;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 8) & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of "shl value, 24" (oracle model: CF = last bit out, AF = 0, OF = bit31 of the last two steps).
static inline uint32_t shl24Flags(uint32_t value) {
    const uint32_t result = value << 24;
    return ((value >> 8) & 1u) | parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((value << 23) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul (one operand) then "shrd eax, edx, 14": eax = bits 14..45 of the 64-bit product, edx = high
// dword. Flags of the shrd (oracle model: CF = bit 13, AF = 0, OF = bit31 of shift-13 ^ shift-14).
static inline uint32_t mulShift14(int32_t a, int32_t b, uint32_t* high, uint32_t* flags) {
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b));
    const uint32_t result = static_cast<uint32_t>(product >> 14);
    const uint32_t previous = static_cast<uint32_t>(product >> 13);
    *high = static_cast<uint32_t>(product >> 32);
    *flags = ((static_cast<uint32_t>(product) >> 13) & 1u) | parityFlag(result)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    return result;
}

// (value - (i16)word) << 24 as a 64-bit dividend, idiv by [0x603854]; flags of the shl (idiv keeps them).
static inline uint32_t scaledRatio(uint32_t value, uint32_t word, uint32_t* edx, uint32_t* flags) {
    const uint32_t diff = value - static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(word)));
    *flags = shl24Flags(diff);
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(diff)) * 0x1000000,
                                         GP4_GLOBAL(int32_t, 0x00603854), &remainder);
    *edx = static_cast<uint32_t>(remainder);
    return static_cast<uint32_t>(quotient);
}

// FUNCTION: GP4 0x004155c7
// esi = car, edi = its track segment. Clears 0x62fbec / 0x62fb49, then (unless bit 4 of +0xe5 is
// set or none of +0x9c bit 0, +0xe5 bit 2, +0x78 bit 0 is) rates the car through 0x415595,
// 0x426b00 and 0x426947 into 0x62fbc8 / 0x62fb49, and stores either the non-negative rating word
// into 0x638dbc or, for a negative rating, the clamped 0..0x4000 product into 0x62fbec.
void __cdecl FUN_004155c7(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const seg = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t flags;

    GP4_GLOBAL(uint32_t, 0x0062fbec) = 0u;
    GP4_GLOBAL(uint8_t, 0x0062fb49) = 0u;

    const uint32_t state = GP4_FIELD(uint8_t, car, 0xe5);
    flags = logicFlags8(state & 0x10u);
    bool active = (state & 0x10u) == 0u;
    bool compute = false;
    if (active) {
        const uint32_t control = GP4_FIELD(uint8_t, car, 0x9c);
        flags = logicFlags8(control & 1u);
        if ((control & 1u) != 0u) {
            compute = true;
        } else {
            const uint32_t again = GP4_FIELD(uint8_t, car, 0xe5);
            flags = logicFlags8(again & 4u);
            if ((again & 4u) == 0u) {
                const uint32_t mode = GP4_FIELD(uint8_t, car, 0x78);
                flags = logicFlags8(mode & 1u);
                if ((mode & 1u) == 0u) active = false;
            }
            if (active) {
                const uint32_t aids = GP4_FIELD(uint8_t, car, 0xce);
                flags = logicFlags8(aids & 4u);
                compute = (aids & 4u) == 0u;
            }
        }
    }

    if (active) {
        if (compute) {
            eax = GP4_FIELD(uint16_t, car, 0xd8);
            flags = subFlags32(eax, 0x6000u);
            if (static_cast<int32_t>(eax) < 0x6000) {
                bool scale = false;
                const uint32_t kind = GP4_FIELD(uint8_t, car, 0x8d);
                flags = logicFlags8(kind & 2u);
                if ((kind & 2u) != 0u) {
                    scale = true;
                } else {
                    const uint32_t b162 = GP4_FIELD(uint8_t, car, 0x162);
                    flags = logicFlags8(b162 & 8u);
                    if ((b162 & 8u) != 0u) {
                        scale = true;
                    } else {
                        const uint32_t b161 = GP4_FIELD(uint8_t, car, 0x161);
                        flags = logicFlags8(b161 & 1u);
                        scale = (b161 & 1u) != 0u;
                    }
                }
                if (scale) {
                    eax = mulShift14(static_cast<int32_t>(eax), GP4_GLOBAL(int32_t, 0x00601e94), &edx, &flags);
                }
            }
            eax = scaledRatio(eax, GP4_FIELD(uint16_t, car, 0x9a), &edx, &flags);
        }

        s.eax = eax;
        s.edx = edx;
        s.eflags = (r->eflags & ~0x8d5u) | flags;
        gp4::call_regs(0x00415595, s);
        eax = s.eax;
        edx = s.edx;

        const uint32_t speed = GP4_FIELD(uint16_t, car, 0x9a);
        flags = subFlags16(speed, 0x3abu);
        bool direct = static_cast<int16_t>(speed) < 0x3ab;
        if (!direct) {
            s.eflags = (r->eflags & ~0x8d5u) | flags;
            gp4::call_regs(0x00426b00, s);
            eax = s.eax;
            edx = s.edx;
            flags = s.eflags & 0x8d5u;
            direct = (flags & 1u) != 0u;
        }

        if (direct) {
            eax = scaledRatio(GP4_GLOBAL(uint16_t, 0x0062d22c), GP4_FIELD(uint16_t, car, 0x9a), &edx, &flags);
            s.eax = eax;
            s.edx = edx;
            s.eflags = (r->eflags & ~0x8d5u) | flags;
            gp4::call_regs(0x00426947, s);
            eax = s.eax;
            edx = s.edx;
            GP4_GLOBAL(uint32_t, 0x0062fbc8) = eax;
            flags = logicFlags32(eax);
            if (static_cast<int32_t>(eax) >= 0) GP4_GLOBAL(uint8_t, 0x0062fb49) = 0xffu;
        } else {
            const uint32_t aids = GP4_FIELD(uint8_t, car, 0xce);
            flags = logicFlags8(aids & 4u);
            bool fixed = false;
            if ((aids & 4u) != 0u) {
                const uint32_t control = GP4_FIELD(uint8_t, car, 0x9c);
                flags = logicFlags8(control & 1u);
                fixed = (control & 1u) == 0u;
            }
            bool limit = true;
            if (fixed) {
                GP4_GLOBAL(uint32_t, 0x0062fbc8) = 0x4000u;
                s.eax = GP4_GLOBAL(uint32_t, 0x006247b8);
                s.edx = edx;
                s.eflags = (r->eflags & ~0x8d5u) | flags;
                gp4::call_regs(0x00426947, s);
                eax = s.eax;
                edx = s.edx;
            } else {
                s.eax = eax;
                s.edx = edx;
                s.eflags = (r->eflags & ~0x8d5u) | flags;
                gp4::call_regs(0x00426947, s);
                GP4_GLOBAL(uint32_t, 0x0062fbc8) = s.eax;
                flags = logicFlags32(s.eax);
                if (static_cast<int32_t>(s.eax) >= 0) GP4_GLOBAL(uint8_t, 0x0062fb49) = 0xffu;
                s.eax = GP4_GLOBAL(uint32_t, 0x006247b8);
                s.eflags = (r->eflags & ~0x8d5u) | flags;
                gp4::call_regs(0x00426947, s);
                eax = s.eax;
                edx = s.edx;
                const uint32_t first = GP4_GLOBAL(uint32_t, 0x0062fbc8);
                flags = subFlags32(eax, first);
                if (static_cast<int32_t>(eax) >= static_cast<int32_t>(first)) {
                    eax = first;
                    limit = false;
                }
            }
            if (limit) {
                const uint32_t edge = GP4_FIELD(uint8_t, seg, 0x51);
                flags = logicFlags8(edge & 0x20u);
                if ((edge & 0x20u) == 0u) {
                    const uint32_t threshold = GP4_GLOBAL(uint32_t, 0x00604118);
                    flags = subFlags32(eax, threshold);
                    if (static_cast<int32_t>(eax) > static_cast<int32_t>(threshold)) {
                        edx = GP4_GLOBAL(uint32_t, 0x00624850);
                        const uint32_t bound = GP4_GLOBAL(uint32_t, 0x00625890);
                        flags = subFlags32(edx, bound);
                        if (static_cast<int32_t>(edx) >= static_cast<int32_t>(bound)) {
                            eax = GP4_GLOBAL(uint32_t, 0x0062fbc8);
                        }
                    }
                }
            }
        }

        flags = logicFlags32(eax);
        if (static_cast<int32_t>(eax) >= 0) {
            const uint32_t extra = GP4_FIELD(uint8_t, car, 0x9d);
            flags = logicFlags8(extra & 0x20u);
            if ((extra & 0x20u) != 0u) {
                edx = GP4_FIELD(uint16_t, car, 0x9a);
                flags = subFlags32(edx, 0x1000u);
                if (static_cast<int32_t>(edx) >= 0x1000) {
                    eax = mulShift14(static_cast<int32_t>(eax), GP4_GLOBAL(int32_t, 0x006039fc), &edx, &flags);
                }
            }
            GP4_GLOBAL(uint16_t, 0x00638dbc) = static_cast<uint16_t>(eax);
        } else {
            if ((GP4_FIELD(uint8_t, car, 0x6f) & 1u) != 0u) {
                GP4_GLOBAL(uint16_t, 0x00638dbc) = 0x8000u;
            }
            GP4_FIELD(uint8_t, car, 0x6f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x6f) & 0xfeu);
            GP4_FIELD(uint16_t, car, 0xee) = 0u;
            GP4_FIELD(uint8_t, car, 0x6f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x6f) | 2u);
            eax = 0u - eax;
            bool steep = false;
            if (GP4_GLOBAL(uint8_t, 0x0062fb49) != 0u) {
                steep = (GP4_FIELD(uint8_t, seg, 0x51) & 0x80u) != 0u;
            }
            const int32_t factor = steep ? GP4_GLOBAL(int32_t, 0x0062fbe4) : GP4_GLOBAL(int32_t, 0x0062fbe8);
            eax = mulShift14(static_cast<int32_t>(eax), factor, &edx, &flags);
            if (static_cast<int32_t>(eax) < 0) eax = 0u;
            flags = subFlags32(eax, 0x4000u);
            if (static_cast<int32_t>(eax) > 0x4000) eax = 0x4000u;
            GP4_GLOBAL(uint32_t, 0x0062fbec) = eax;
        }
    }

    r->eax = eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = edx;
    r->ebp = s.ebp;
    r->esi = s.esi;
    r->edi = s.edi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004155c7, FUN_004155c7, "regs(esi:ptr[0x420]:layout=car_004129fa+rec_00412b88+rec_00426947, edi:rec[0x8943a0,0x160,0..26], eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0+rel_0042c0c4+state_0043c677+pit_0040d9e6+trk_007ad894+state_00427880+state_00426947+frac_00624790+state_004155c7")
