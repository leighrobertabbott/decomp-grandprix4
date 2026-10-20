#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte logic result (and / or): CF = OF = AF = 0.
static inline uint32_t logic8Flags(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// Flags of `sar r32, 16` in the oracle model: CF = last bit shifted out, OF = AF = 0.
static inline uint32_t sar16Flags(uint32_t before, uint32_t result) {
    return ((before >> 15) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// One-operand `imul` followed by `shrd eax, edx, 14`; edx keeps the high half.
static inline uint32_t mulShr14(uint32_t a, uint32_t b, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a))
        * static_cast<int64_t>(static_cast<int32_t>(b));
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

static inline uint8_t& byteAt(uint8_t* p, uint32_t offset) { return GP4_FIELD(uint8_t, p, offset); }

// FUNCTION: GP4 0x00426b30
// Longitudinal speed update of the car at esi (segment edi): the signed speed +0x98 (sign in bit 2 of
// +0x71) is advanced by the drive/drag term [0x603804] (4*[0x603d48] - 0x00426a66's [0x6037fc], then the
// 0x00425fdc braking term or the 0x00426947 / 0x00426aa5 / 0x00425eca throttle term in [0x603808]),
// clamped to the current speed, after 0x004266dc (limit +0xd8) and 0x004257c9; +0xda receives the change
// ((new speed or segment word +0xca) - old speed) >> 16. The flags are the last flag writer's.
void __cdecl FUN_00426b30(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    uint32_t flags;

    GP4_FIELD(uint16_t, car, 0x24a) = 0u;
    s.eax = GP4_FIELD(uint32_t, car, 0x98);
    if ((byteAt(car, 0x71) & 4u) != 0u) {
        s.eax = 0u - s.eax;
    }
    GP4_GLOBAL(uint32_t, 0x006037f8) = s.eax;
    GP4_GLOBAL(uint32_t, 0x00603804) = 0u;
    GP4_GLOBAL(uint32_t, 0x00603808) = 0u;
    gp4::call_regs(0x00426a66, s);

    s.edx = GP4_GLOBAL(uint32_t, 0x00603d48) << 2;
    s.eax = (0u - GP4_GLOBAL(uint32_t, 0x006037fc)) + s.edx;
    GP4_GLOBAL(uint32_t, 0x00603804) += s.eax;
    {
        int32_t remainder;
        const int32_t quotient = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(s.eax)) * 256,
                                             GP4_GLOBAL(int32_t, 0x00603854), &remainder);
        s.eax = static_cast<uint32_t>(quotient);
        s.edx = static_cast<uint32_t>(remainder);
    }
    GP4_GLOBAL(uint32_t, 0x006247b4) = s.eax;
    gp4::call_regs(0x004266dc, s);

    s.eax = 0x1000u;
    if ((byteAt(car, 0x9c) & 1u) != 0u || (byteAt(car, 0xce) & 4u) == 0u) {
        const uint16_t limit = static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0xd8) - GP4_GLOBAL(uint16_t, 0x00603d3a));
        s.eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(limit)));
    }
    GP4_GLOBAL(uint32_t, 0x00934df4) = s.eax;
    gp4::call_regs(0x004257c9, s);

    {
        int32_t remainder;
        const int32_t quotient = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(int16_t, 0x00934df4)) * 0x1000000,
                                             GP4_GLOBAL(int32_t, 0x00603854), &remainder);
        s.eax = static_cast<uint32_t>(quotient);
        s.edx = static_cast<uint32_t>(remainder);
    }
    if (static_cast<int32_t>(s.eax) >= GP4_GLOBAL(int32_t, 0x006247b8) && GP4_FIELD(int16_t, car, 0x9a) >= 0x3ab) {
        gp4::call_regs(0x00426ada, s);
        if ((s.eflags & 1u) == 0u) {
            s.eax = GP4_GLOBAL(uint32_t, 0x006247b8);
        }
    }
    gp4::call_regs(0x00426947, s);

    bool stopped = false;
    if (static_cast<int32_t>(s.eax) >= 0) {
        GP4_FIELD(uint16_t, car, 0x24a) = 0u;
        if ((byteAt(car, 0xfe) & 0x20u) == 0u && (byteAt(car, 0x162) & 1u) != 0u) {
            stopped = true;
        } else {
            if ((byteAt(car, 0x8d) & 4u) != 0u) {
                s.edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x9a)));
                if (static_cast<int32_t>(s.edx) >= GP4_GLOBAL(int32_t, 0x00603d58)) {
                    s.edx = GP4_GLOBAL(uint32_t, 0x00603d5c);
                    if (static_cast<int32_t>(s.eax) > static_cast<int32_t>(s.edx)) {
                        s.eax = s.edx;
                    }
                }
            }
            if ((byteAt(car, 0xca) & 0x20u) != 0u && (byteAt(car, 0xcc) & 8u) != 0u) {
                if (static_cast<int32_t>(s.eax) > GP4_GLOBAL(int32_t, 0x00603d54)) {
                    s.eax = GP4_GLOBAL(uint32_t, 0x00603d54);
                }
            }
            if ((byteAt(car, 0x230) & 8u) != 0u) {
                s.eax = 0u;
            }
            if ((byteAt(car, 0x162) & 0x80u) != 0u) {
                s.eax = 0u;
            }
            if (GP4_FIELD(uint32_t, car, 0x240) != 0u && (byteAt(car, 0xe5) & 0x80u) != 0u) {
                s.eax = 0u;
            }
            gp4::call_regs(0x00426aa5, s);
            GP4_FIELD(uint16_t, car, 0xee) = static_cast<uint16_t>(s.eax);
            if ((byteAt(car, 0x162) & 1u) != 0u) {
                stopped = true;
            } else {
                if ((byteAt(car, 0x203) & 0x80u) != 0u) {
                    s.edx = GP4_FIELD(uint16_t, car, 0x238);
                    s.eax = mulShr14(s.eax, s.edx, &s.edx);
                }
                if ((byteAt(car, 0x203) & 0x10u) != 0u) {
                    s.eax = mulShr14(s.eax, GP4_GLOBAL(uint32_t, 0x006029ec), &s.edx);
                }
                if ((byteAt(car, 0xca) & 8u) != 0u) {
                    s.eax = mulShr14(s.eax, GP4_GLOBAL(uint32_t, 0x00604168), &s.edx);
                }
                GP4_GLOBAL(uint32_t, 0x00603d40) = s.eax;
                gp4::call_regs(0x00425eca, s);
                s.edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x24c))) << 14;
                if ((byteAt(car, 0x78) & 0x40u) != 0u) {
                    uint32_t high;
                    s.edx = mulShr14(GP4_GLOBAL(uint32_t, 0x006039dc), s.edx, &high);
                }
                if (static_cast<int32_t>(s.eax) > static_cast<int32_t>(s.edx)) {
                    s.eax = s.edx;
                }
                s.eax <<= 2;
                GP4_GLOBAL(uint32_t, 0x00603808) = s.eax;
                if ((byteAt(car, 0x203) & 0x80u) != 0u) {
                    s.eax = (s.eax & 0xffff0000u) | GP4_FIELD(uint16_t, car, 0x9a);
                    if (GP4_FIELD(int16_t, car, 0x9a) >= 0xcd5) {
                        byteAt(car, 0x203) = static_cast<uint8_t>(byteAt(car, 0x203) & 0x7fu);
                    }
                }
            }
        }
    } else {
        s.eax = 0u - s.eax;
        GP4_FIELD(uint16_t, car, 0x24a) = static_cast<uint16_t>(s.eax);
        s.eax = 0u;
        gp4::call_regs(0x00426aa5, s);
        GP4_FIELD(uint16_t, car, 0xee) = static_cast<uint16_t>(s.eax);
        s.eax = GP4_FIELD(uint16_t, car, 0x24a);
        if (static_cast<int32_t>(s.eax) >= GP4_GLOBAL(int32_t, 0x006247d0)) {
            byteAt(car, 0x9c) = static_cast<uint8_t>(byteAt(car, 0x9c) | 0x40u);
        }
        gp4::call_regs(0x00425fdc, s);
        s.edx = (0u - static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x24e)))) << 14;
        if (static_cast<int32_t>(s.eax) < static_cast<int32_t>(s.edx)) {
            s.eax = s.edx;
        }
        s.eax <<= 2;
        GP4_GLOBAL(uint32_t, 0x00603804) += s.eax;
    }

    if (stopped) {
        GP4_FIELD(uint32_t, car, 0x98) = 0u;
        GP4_FIELD(uint16_t, car, 0xda) = 0u;
        byteAt(car, 0x163) = static_cast<uint8_t>(byteAt(car, 0x163) & 0xfeu);
        flags = logic8Flags(byteAt(car, 0x163));
    } else {
        uint16_t gap = static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x9a) - GP4_FIELD(uint16_t, car, 0xd8));
        if (static_cast<int16_t>(gap) < 0) {
            gap = static_cast<uint16_t>(0u - gap);
        }
        s.eax = (s.eax & 0xffff0000u) | gap;
        if (static_cast<int16_t>(gap) > GP4_GLOBAL(int16_t, 0x00603d68)) {
            byteAt(car, 0x163) = static_cast<uint8_t>(byteAt(car, 0x163) & 0xfeu);
        } else {
            byteAt(car, 0x163) = static_cast<uint8_t>(byteAt(car, 0x163) | 1u);
        }

        s.edx = static_cast<uint32_t>(GP4_FIELD(uint16_t, car, 0x176)) << 12;
        GP4_GLOBAL(uint32_t, 0x00603804) -= s.edx;
        s.eax = GP4_FIELD(uint32_t, car, 0x98);
        if ((byteAt(car, 0x71) & 4u) != 0u) {
            s.eax = 0u - s.eax;
            GP4_GLOBAL(uint32_t, 0x00603804) = 0u - GP4_GLOBAL(uint32_t, 0x00603804);
        }
        s.edx = GP4_FIELD(uint32_t, car, 0x98);
        uint32_t change = GP4_GLOBAL(uint32_t, 0x00603804);
        if (static_cast<int32_t>(change) < 0) {
            uint32_t magnitude = 0u - change;
            if (magnitude >= s.edx) {
                magnitude = s.edx;
            }
            change = 0u - magnitude;
        } else if (change >= s.edx) {
            change = s.edx;
        }
        GP4_GLOBAL(uint32_t, 0x00603804) = change;

        s.eax = s.eax + change + GP4_GLOBAL(uint32_t, 0x00603808);
        byteAt(car, 0x71) = static_cast<uint8_t>(byteAt(car, 0x71) & 0xfbu);
        s.edx = s.eax;
        if (static_cast<int32_t>(s.eax) < 0) {
            byteAt(car, 0x71) = static_cast<uint8_t>(byteAt(car, 0x71) | 4u);
            s.edx = 0u - s.edx;
        }
        GP4_FIELD(uint32_t, car, 0x98) = s.edx;
        if (GP4_GLOBAL(uint8_t, 0x00625a4f) != 0u) {
            uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(s.edi));
            s.edx = GP4_FIELD(uint16_t, segment, 0xca);
        }
        const uint32_t delta = s.edx - GP4_GLOBAL(uint32_t, 0x006037f8);
        s.edx = static_cast<uint32_t>(static_cast<int32_t>(delta) >> 16);
        flags = sar16Flags(delta, s.edx);
        GP4_FIELD(uint16_t, car, 0xda) = static_cast<uint16_t>(s.edx);
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->ebp = s.ebp;
    r->edi = s.edi;
    r->eflags = (s.eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426b30, FUN_00426b30, "regs(ebx:u32, ecx:u32, ebp:u32, esi:ptr[0x420]:layout=car_0042ba87+rec_00409416+car_00429fcd+gears_0041034e+rec_0043259c+rec_00426091+car_00425473+car_004149f4+rec_00426947+car_0042b17e+car_00429c7f+car_0043c677, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+state_0042595c+ring_008943a0+words_008943a0+words_00413090+engine_00413606+lanes_00429fcd+surf_0041f02b+state_0043c1ba+state_004320f2+counter_0040f0dd+scale_0066ec60+state_00432330+state_004154d5+ring_0040c0ae+flags_0042aa47+state_0040ee7b+state_00426091+cache_00426091+state_00425473+opt_00625a53+state_00426947+frac_00624790+state_0042882e")
