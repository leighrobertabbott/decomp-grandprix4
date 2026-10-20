#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// One-operand `imul dword` followed by `shrd eax, edx, 14`: bits 14..45 of the signed product,
// edx keeps the high half.
static inline uint32_t mulShr14(uint32_t a, uint32_t b, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a))
        * static_cast<int64_t>(static_cast<int32_t>(b));
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// eax = ((eax << 14) - word +0x66) * [0x62a94c] >> 14, plus the offset global; ecx = min(ecx, eax).
static inline void capByRange(gp4::Regs& s, uint8_t* car, uint32_t offset) {
    uint32_t value = s.eax << 14;
    s.edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x66)));
    value -= s.edx;
    uint32_t high;
    value = mulShr14(value, GP4_GLOBAL(uint32_t, 0x0062a94c), &high);
    s.edx = high;
    s.eax = value + offset;
    if (static_cast<int32_t>(s.ecx) > static_cast<int32_t>(s.eax)) {
        s.ecx = s.eax;
    }
}

// FUNCTION: GP4 0x004266dc
// Speed-limit update of the car at esi on segment edi: the limit (ecx) comes from 0x0042637c,
// 0x004265a9 (after 0x00425cb3 when the lane byte of the segment block changes, storing its dx:ax
// at +0xbc) or the word [0x0062d22c], optionally scaled by [0x006029e8], capped by the range
// tests 0x00432330 / 0x004322d9 and the grid limits, by the words +0x100 / +0xd8, raised to 0x3ab
// and capped by [0x00603d60]-scaled minimum [0x00603d64]; stored at word +0xd8.
void __cdecl FUN_004266dc(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t segmentAddress = r->edi;
    const uint8_t* segment = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(segmentAddress));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    const bool special = (GP4_FIELD(uint8_t, car, 0xe5) & 4u) != 0u
        || GP4_GLOBAL(uint8_t, 0x007c2e75) != 0u
        || GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0u;

    bool checkSkip = true;
    bool direct = false;
    if (special) {
        if ((GP4_FIELD(uint8_t, car, 0x8d) & 2u) != 0u) {
            GP4_FIELD(uint8_t, car, 0x9c) |= 1u;
            checkSkip = false;
        } else if ((segment[0x92] & 1u) != 0u && (GP4_FIELD(uint8_t, car, 0x71) & 0x80u) != 0u) {
            GP4_FIELD(uint8_t, car, 0x9c) |= 1u;
            s.ecx = GP4_GLOBAL(uint16_t, 0x0062d22c);
            GP4_FIELD(uint32_t, car, 0xbc) = 0u;
            checkSkip = false;
            direct = true;
        }
    }
    if (checkSkip && (GP4_FIELD(uint8_t, car, 0xce) & 4u) != 0u) {
        r->eflags &= ~0x8d5u;                       // test result 4: all six flags clear
        return;
    }

    bool scale = true;
    if (!direct) {
        s.ebx = GP4_FIELD(uint32_t, car, 0x1b8);
        bool lanePath = true;
        if ((GP4_FIELD(uint8_t, car, 0x8d) & 2u) != 0u) {
            if (GP4_GLOBAL(uint8_t, 0x007c2e75) == 0u && (GP4_FIELD(uint8_t, car, 0x26b) & 0x80u) != 0u) {
                GP4_FIELD(uint32_t, car, 0xbc) = 0u;
                gp4::call_regs(0x0042637c, s);
                lanePath = false;
                scale = false;
            } else {
                s.ebx = GP4_FIELD(uint32_t, car, 0x1c);
            }
        }
        if (lanePath) {
            const uint8_t* block = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(s.ebx));
            uint8_t lane = block[0x2a];
            s.eax = (s.eax & ~0xffu) | lane;
            if (lane != GP4_FIELD(uint8_t, car, 0x234)) {
                bool changed = true;
                if ((segment[0x51] & 0x80u) != 0u) {
                    lane = block[0x8d];
                    s.eax = (s.eax & ~0xffu) | lane;
                    changed = lane != GP4_FIELD(uint8_t, car, 0x234);
                }
                if (changed) {
                    GP4_FIELD(uint8_t, car, 0x234) = lane;
                    GP4_GLOBAL(uint8_t, 0x00603e6c) = 0xffu;
                    const uint32_t savedBlock = s.ebx;
                    gp4::call_regs(0x00425cb3, s);
                    s.ebx = savedBlock;
                    GP4_FIELD(uint16_t, car, 0xbc) = static_cast<uint16_t>(s.eax);
                    GP4_FIELD(uint16_t, car, 0xbe) = static_cast<uint16_t>(s.edx);
                    GP4_FIELD(uint16_t, car, 0x13e) = 0u;
                }
            }
            gp4::call_regs(0x004265a9, s);
        }
    }

    if (scale && (GP4_FIELD(uint8_t, car, 0x203) & 0x10u) != 0u) {
        uint32_t high;
        s.eax = mulShr14(s.ecx, GP4_GLOBAL(uint32_t, 0x006029e8), &high);
        s.edx = high;
        s.ecx = s.eax;
    }

    if (GP4_GLOBAL(uint8_t, 0x007c2e75) != 0u || GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0u) {
        if (segmentAddress < GP4_GLOBAL(uint32_t, 0x00868518)) {
            if ((GP4_FIELD(uint8_t, car, 0x71) & 0x80u) != 0u
                && (segmentAddress >= GP4_GLOBAL(uint32_t, 0x00891500)
                    || (GP4_FIELD(uint8_t, car, 0x1ce) & 1u) != 0u)) {
                gp4::call_regs(0x00432330, s);
                if ((s.eflags & 1u) == 0u) {
                    capByRange(s, car, GP4_GLOBAL(uint32_t, 0x0062a958));
                }
            }
        } else if (segmentAddress < GP4_GLOBAL(uint32_t, 0x0086851c)) {
            GP4_FIELD(uint8_t, car, 0x9d) |= 0x20u;
            if (static_cast<int32_t>(s.ecx) > GP4_GLOBAL(int32_t, 0x0062a958)) {
                s.ecx = GP4_GLOBAL(uint32_t, 0x0062a958);
            }
        }
    }

    if (GP4_GLOBAL(uint8_t, 0x007c2e75) == 0u
        && (GP4_FIELD(uint8_t, car, 0x71) & 0x80u) != 0u
        && (segmentAddress >= GP4_GLOBAL(uint32_t, 0x00891500)
            || (GP4_FIELD(uint8_t, car, 0x1ce) & 1u) != 0u)) {
        gp4::call_regs(0x004322d9, s);
        if ((s.eflags & 1u) == 0u) {
            capByRange(s, car, GP4_GLOBAL(uint32_t, 0x0062a948));
        }
    }

    uint32_t cx = s.ecx;
    if ((GP4_FIELD(uint8_t, car, 0xe5) & 4u) != 0u && (segment[0x51] & 0x80u) != 0u) {
        if ((GP4_FIELD(uint8_t, car, 0x135) & 2u) != 0u) {
            if (static_cast<int16_t>(cx) > GP4_FIELD(int16_t, car, 0x100)) {
                cx = (cx & 0xffff0000u) | GP4_FIELD(uint16_t, car, 0x100);
            }
        } else {
            bool cap = (GP4_FIELD(uint8_t, car, 0xfe) & 0x40u) != 0u
                || (GP4_FIELD(uint8_t, car, 0x203) & 1u) != 0u
                || (GP4_FIELD(uint8_t, car, 0x179) & 4u) != 0u;
            if (!cap) {
                s.eax = GP4_FIELD(uint32_t, car, 0x1a0);
                cap = static_cast<int32_t>(s.eax) < GP4_GLOBAL(int32_t, 0x00603e1c);
            }
            if (cap && static_cast<int16_t>(cx) > GP4_FIELD(int16_t, car, 0xd8)) {
                cx = (cx & 0xffff0000u) | GP4_FIELD(uint16_t, car, 0xd8);
            }
        }
    }

    int32_t limit = static_cast<int16_t>(cx);
    if (limit < 0x3ab) {
        limit = 0x3ab;
    }
    s.ecx = static_cast<uint32_t>(limit);

    uint32_t flags = 0x44u;                         // a zero `test` result
    if ((GP4_FIELD(uint8_t, car, 0xe5) & 4u) != 0u && (GP4_FIELD(uint8_t, car, 0x8d) & 4u) != 0u) {
        uint32_t high;
        s.eax = mulShr14(s.ecx, GP4_GLOBAL(uint32_t, 0x00603d60), &high);
        s.edx = high;
        if (static_cast<int32_t>(s.eax) < GP4_GLOBAL(int32_t, 0x00603d64)) {
            s.eax = GP4_GLOBAL(uint32_t, 0x00603d64);
        }
        flags = subFlags32(s.ecx, s.eax);
        if (static_cast<int32_t>(s.ecx) > static_cast<int32_t>(s.eax)) {
            s.ecx = s.eax;
        }
    }
    GP4_FIELD(uint16_t, car, 0xd8) = static_cast<uint16_t>(s.ecx);

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->ebp = s.ebp;
    r->eflags = (s.eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004266dc, FUN_004266dc, "regs(eax:u32, edx:u32, esi:ptr[0x420]:layout=car_0042ba87+rec_00409416+car_00429fcd+gears_0041034e+rec_0043259c+car_0042b17e+car_00429c7f+car_0043c677, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+state_0042595c+ring_008943a0+words_008943a0+words_00413090+engine_00413606+lanes_00429fcd+surf_0041f02b+state_0043c1ba+state_004320f2+counter_0040f0dd+scale_0066ec60+state_00432330+state_004154d5+ring_0040c0ae+flags_0042aa47+state_0040ee7b")
