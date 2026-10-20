#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right" / "sub": CF, PF, AF, ZF, SF, OF
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// edx:eax / divisor as the x86 "div" does when edx < divisor (otherwise the CPU faults).
static inline void div64by32(uint32_t hi, uint32_t lo, uint32_t divisor, uint32_t* quotient, uint32_t* remainder) {
    uint32_t rem = hi;
    uint32_t quot = 0u;
    for (int bit = 31; bit >= 0; bit--) {
        const uint32_t carry = rem >> 31;
        rem = (rem << 1) | ((lo >> bit) & 1u);
        quot <<= 1;
        if (carry != 0u || rem >= divisor) {
            rem -= divisor;
            quot |= 1u;
        }
    }
    *quotient = quot;
    *remainder = rem;
}

// FUNCTION: GP4 0x004192a8
// Returns eax = signed ratio |a * k| * |b| / |c| (clamped to 0x200, sign from the three operands)
// with CF set when the clamped magnitude is below the threshold [0x6034c0].
void __cdecl FUN_004192a8(gp4::Regs* r) {
    // eax = ([0x6036a8] - [0x6036ac]) * [0x60368c] >> 14
    const uint32_t delta = GP4_GLOBAL(uint32_t, 0x006036a8) - GP4_GLOBAL(uint32_t, 0x006036ac);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(delta)) * GP4_GLOBAL(int32_t, 0x0060368c);
    uint32_t eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    uint32_t edx = GP4_GLOBAL(uint32_t, 0x006036b4);
    uint32_t ecx = GP4_GLOBAL(uint32_t, 0x006036b0);
    const uint32_t ebp = eax ^ edx ^ ecx;

    if ((eax & 0x80000000u) != 0u) eax = 0u - eax;
    if ((edx & 0x80000000u) != 0u) edx = 0u - edx;
    if ((ecx & 0x80000000u) != 0u) ecx = 0u - ecx;

    // mul edx
    const uint64_t wide = static_cast<uint64_t>(eax) * edx;
    eax = static_cast<uint32_t>(wide);
    edx = static_cast<uint32_t>(wide >> 32);

    // keep the divisor above the high half so div cannot overflow
    if (!(ecx > edx)) {
        ecx = edx + 1u;
        if (ecx == 0u) {
            ecx = 0xffffffffu;
            edx = edx - 1u;
        }
    }

    // div ecx
    uint32_t quotient;
    uint32_t remainder;
    div64by32(edx, eax, ecx, &quotient, &remainder);
    eax = quotient;
    edx = remainder;

    if (eax >= 0x200u) {
        eax = 0x200u;
    }

    const uint32_t threshold = GP4_GLOBAL(uint32_t, 0x006034c0);
    uint32_t flags;
    if (static_cast<int32_t>(eax) < static_cast<int32_t>(threshold)) {
        // stc after "cmp eax, [0x6034c0]"
        flags = subFlags(eax, threshold) | 1u;
    } else if ((ebp & 0x80000000u) == 0u) {
        // clc after "test ebp, ebp"
        flags = parityFlag(ebp) | (ebp == 0u ? 0x40u : 0u);
    } else {
        // neg eax, then clc
        const uint32_t negated = 0u - eax;
        flags = parityFlag(negated)
            | (((0u ^ eax ^ negated) & 0x10u) != 0u ? 0x10u : 0u)
            | (negated == 0u ? 0x40u : 0u)
            | ((negated & 0x80000000u) != 0u ? 0x80u : 0u)
            | (eax == 0x80000000u ? 0x800u : 0u);
        eax = negated;
    }

    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004192a8, FUN_004192a8, "regs() -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
