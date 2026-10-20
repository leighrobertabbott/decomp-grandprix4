#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of "neg value".
static inline uint32_t negFlags(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

struct Divisor {
    int32_t value;
    uint32_t flags;   // flags of the last neg / cmp that produced it
};

// Sign-preserving clamp of the magnitude to at least 0x240 (neg; cmp; mov; neg on negatives, cmp; mov
// on the rest).
static inline Divisor clampDivisor(int32_t v) {
    Divisor d;
    if (v < 0) {
        int32_t magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(v));
        if (magnitude < 0x240) magnitude = 0x240;
        d.value = static_cast<int32_t>(0u - static_cast<uint32_t>(magnitude));
        d.flags = negFlags(static_cast<uint32_t>(magnitude));
    } else {
        d.flags = cmpFlags(static_cast<uint32_t>(v), 0x240u);
        d.value = v < 0x240 ? 0x240 : v;
    }
    return d;
}

// FUNCTION: GP4 0x00427bc1
// edi = track-segment record, eax = signed offset. From the pit start [0x891500] on, returns the mean
// of the signed words +0x52 / +0x54. Before it, eax < 0 uses word +0x52 and zero-extended word
// +0x60, otherwise word +0x54 and word +0x62: 0x1000000 / clamp(word) -/+ (word + 0x50) - eax is
// clamped the same way and divides 0x1000000 again (eax = quotient, edx = remainder, flags of the
// clamp since idiv leaves them). ebp and ecx are preserved.
void __cdecl FUN_00427bc1(gp4::Regs* r) {
    uint8_t* const seg = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    if (r->edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        const int32_t first = static_cast<int32_t>(GP4_FIELD(int16_t, seg, 0x52));
        const int32_t sum = static_cast<int32_t>(GP4_FIELD(int16_t, seg, 0x54)) + first;
        const int32_t mean = sum >> 1;
        const uint32_t result = static_cast<uint32_t>(mean);
        const uint32_t flags = (static_cast<uint32_t>(sum) & 1u) | parityFlag(result)
            | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
        r->eax = result;
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    const int32_t offset = static_cast<int32_t>(r->eax);
    int32_t remainder;
    int32_t estimate;
    if (offset < 0) {
        const Divisor d = clampDivisor(static_cast<int32_t>(GP4_FIELD(int16_t, seg, 0x52)));
        const int32_t quotient = gp4::idiv64(0x1000000, d.value, &remainder);
        const uint32_t bias = static_cast<uint32_t>(GP4_FIELD(uint16_t, seg, 0x60)) + 0x50u;
        estimate = static_cast<int32_t>(static_cast<uint32_t>(quotient) - bias);
    } else {
        const Divisor d = clampDivisor(static_cast<int32_t>(GP4_FIELD(int16_t, seg, 0x54)));
        const int32_t quotient = gp4::idiv64(0x1000000, d.value, &remainder);
        const uint32_t bias = static_cast<uint32_t>(GP4_FIELD(uint16_t, seg, 0x62)) + 0x50u;
        estimate = static_cast<int32_t>(static_cast<uint32_t>(quotient) + bias);
    }

    const int32_t difference = static_cast<int32_t>(static_cast<uint32_t>(estimate) - static_cast<uint32_t>(offset));
    const Divisor d = clampDivisor(difference);
    const int32_t result = gp4::idiv64(0x1000000, d.value, &remainder);
    r->eax = static_cast<uint32_t>(result);
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = (r->eflags & ~0x8d5u) | d.flags;
}
GP4_IMPL(0x00427bc1, FUN_00427bc1, "regs(eax:i32[-4096..4096], edi:rec[0x8943a0,0x160,0..29], edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+words_008943a0+segw_00427bc1; cases=1000")
