#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `sar value, 16`: CF = bit 15 shifted out last, AF = OF = 0 (pinned oracle model).
static inline uint32_t sar16Flags(uint32_t value) {
    const uint32_t result = static_cast<uint32_t>(static_cast<int32_t>(value) >> 16);
    return ((value >> 15) & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00426e8f
// Longitudinal speed update of the car in esi (edi = its track segment): the engine-model speed
// delta from 0x00426a66 plus the drive term [0x603d48] * 4 is turned into the Q8 ratio [0x6247b4]
// (divided by the tick scale [0x603854]); the segment's word +0xa6 less [0x603d3a] gives a second
// ratio (clamped to [0x6247b8]) that 0x00426947 maps to a signed throttle/brake fraction. A positive
// fraction is stored at +0xee (and runs 0x00425eca with its mean with [0x603d3c]), a negative one at
// +0x24a (and runs 0x00425fdc); the result times 4 accumulates in [0x603800], which advances the
// speed word +0x98 (not below zero). The change of the speed >> 16 goes to word +0xda. eax is that
// value, edx what the last callee left, and the flags are those of the final `sar eax, 16`.
void __cdecl FUN_00426e8f(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_FIELD(uint16_t, car, 0x24a) = 0;
    const uint32_t speedBefore = GP4_FIELD(uint32_t, car, 0x98);
    GP4_GLOBAL(uint32_t, 0x006037f8) = speedBefore;
    GP4_GLOBAL(uint32_t, 0x00603800) = 0;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = speedBefore;
    gp4::call_regs(0x00426a66, c);

    // eax = drive term - [0x6037fc]; ratio = (eax << 8) / [0x603854]
    const uint32_t delta = (0u - GP4_GLOBAL(uint32_t, 0x006037fc)) + (GP4_GLOBAL(uint32_t, 0x00603d48) << 2);
    GP4_GLOBAL(uint32_t, 0x00603800) += delta;
    int32_t remainder = 0;
    const int32_t ratio = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(delta)) * 256, GP4_GLOBAL(int32_t, 0x00603854), &remainder);
    GP4_GLOBAL(uint32_t, 0x006247b4) = static_cast<uint32_t>(ratio);

    // segment word +0xa6 less [0x603d3a], sign-extended, scaled by 2^24 and divided by [0x603854]
    const int16_t lift = static_cast<int16_t>(GP4_FIELD(uint16_t, segment, 0xa6) - GP4_GLOBAL(uint16_t, 0x00603d3a));
    GP4_GLOBAL(uint32_t, 0x00934df4) = static_cast<uint32_t>(static_cast<int32_t>(lift));
    int32_t second = gp4::idiv64(static_cast<int64_t>(lift) * 0x1000000, GP4_GLOBAL(int32_t, 0x00603854), &remainder);
    const int32_t cap = GP4_GLOBAL(int32_t, 0x006247b8);
    if (second > cap) {
        second = cap;
    }

    c.eax = static_cast<uint32_t>(second);
    c.edx = static_cast<uint32_t>(remainder);
    gp4::call_regs(0x00426947, c);

    const int32_t fraction = static_cast<int32_t>(c.eax);
    if (fraction < 0) {
        const uint32_t magnitude = 0u - c.eax;
        GP4_FIELD(uint16_t, car, 0x24a) = static_cast<uint16_t>(magnitude);
        GP4_FIELD(uint16_t, car, 0xee) = 0;
        c.eax = magnitude;
        gp4::call_regs(0x00425fdc, c);
    } else {
        GP4_FIELD(uint16_t, car, 0xee) = static_cast<uint16_t>(c.eax);
        GP4_FIELD(uint16_t, car, 0x24a) = 0;
        int32_t other = GP4_GLOBAL(int32_t, 0x00603d3c);
        if (other > fraction) {
            other = fraction;
        }
        const int32_t mean = static_cast<int32_t>(static_cast<uint32_t>(fraction) + static_cast<uint32_t>(other)) >> 1;
        GP4_GLOBAL(uint32_t, 0x00603d40) = static_cast<uint32_t>(mean);
        c.eax = static_cast<uint32_t>(mean);
        c.edx = static_cast<uint32_t>(other);
        gp4::call_regs(0x00425eca, c);
    }
    GP4_GLOBAL(uint32_t, 0x00603800) += c.eax << 2;

    int32_t speed = static_cast<int32_t>(GP4_FIELD(uint32_t, car, 0x98) + GP4_GLOBAL(uint32_t, 0x00603800));
    if (speed < 0) {
        speed = 0;
    }
    GP4_FIELD(uint32_t, car, 0x98) = static_cast<uint32_t>(speed);
    const uint32_t change = static_cast<uint32_t>(speed) - GP4_GLOBAL(uint32_t, 0x006037f8);
    const uint32_t high = static_cast<uint32_t>(static_cast<int32_t>(change) >> 16);
    GP4_FIELD(uint16_t, car, 0xda) = static_cast<uint16_t>(high);

    r->eax = high;
    r->edx = c.edx;
    r->eflags = (r->eflags & ~0x8d5u) | sar16Flags(change);
}
GP4_IMPL(0x00426e8f, FUN_00426e8f, "regs(esi:ptr[0x420]:layout=car_004129fa+rec_00426947, edi:ptr[0xb0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00427880+state_00426947+frac_00624790+gains_00426e8f")
