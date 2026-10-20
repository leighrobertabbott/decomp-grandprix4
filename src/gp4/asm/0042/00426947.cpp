#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `left - right` (cmp / neg).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00426947
// eax = a 0..0x4000 style ratio of (eax - dword 0x6247b4) against thresholds taken from the speed
// tables 0x639654 (positive side), 0x63acd4 / 0x63a194 (negative side). The table index is
// |dword +0x98| / 4 clamped to 0x16600000, >> 20, plus 0x168 when bit 0 of byte +0x8d is set; every
// threshold is table * word +0xd6 >> 6 (the positive side then times the signed word +0xf6 >> 14).
// ebp, ecx and edi are saved; edx is left as the high half of the last imul or the idiv remainder;
// the flags are those of the last compare / add / neg.
void __cdecl FUN_00426947(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t index = GP4_FIELD(uint32_t, body, 0x98);
    if (static_cast<int32_t>(index) < 0) {
        index = 0u - index;
    }
    index >>= 2;
    if (static_cast<int32_t>(index) > 0x16600000) {
        index = 0x16600000u;
    }
    index >>= 0x14;
    if ((GP4_FIELD(uint8_t, body, 0x8d) & 1u) != 0u) {
        index += 0x168u;
    }

    const int32_t diff = static_cast<int32_t>(r->eax - GP4_GLOBAL(uint32_t, 0x006247b4));
    const int32_t scale = static_cast<int32_t>(GP4_FIELD(uint16_t, body, 0xd6));
    uint32_t eax;
    uint32_t edx;
    uint32_t flags;

    if (diff >= 0) {
        const int64_t first = static_cast<int64_t>(GP4_ARRAY(int32_t, 0x00639654)[index]) * static_cast<int64_t>(scale);
        const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 6);
        const int32_t factor = static_cast<int32_t>(GP4_FIELD(int16_t, body, 0xf6));
        const int64_t second = static_cast<int64_t>(static_cast<int32_t>(scaled)) * static_cast<int64_t>(factor);
        const uint32_t limit = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14);
        flags = subFlags32(static_cast<uint32_t>(diff), limit);
        if (diff < static_cast<int32_t>(limit)) {
            const int32_t base = GP4_GLOBAL(int32_t, 0x00624790);
            const int32_t span = 0x4000 - base;
            int32_t remainder;
            const int32_t quotient = gp4::idiv64(static_cast<int64_t>(diff) * static_cast<int64_t>(span), static_cast<int32_t>(limit), &remainder);
            eax = static_cast<uint32_t>(quotient) + static_cast<uint32_t>(base);
            flags = addFlags32(static_cast<uint32_t>(quotient), static_cast<uint32_t>(base));
            edx = static_cast<uint32_t>(remainder);
        } else {
            eax = 0x4000u;
            edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
        }
    } else {
        const int64_t first = static_cast<int64_t>(GP4_ARRAY(int32_t, 0x0063acd4)[index]) * static_cast<int64_t>(scale);
        const uint32_t upper = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 6);
        flags = subFlags32(static_cast<uint32_t>(diff), upper);
        if (diff > static_cast<int32_t>(upper)) {
            const int32_t base = GP4_GLOBAL(int32_t, 0x00624790);
            const int32_t span = -base;
            int32_t remainder;
            const int32_t quotient = gp4::idiv64(static_cast<int64_t>(diff) * static_cast<int64_t>(span), static_cast<int32_t>(upper), &remainder);
            eax = static_cast<uint32_t>(quotient) + static_cast<uint32_t>(base);
            flags = addFlags32(static_cast<uint32_t>(quotient), static_cast<uint32_t>(base));
            edx = static_cast<uint32_t>(remainder);
        } else {
            const int64_t second = static_cast<int64_t>(GP4_ARRAY(int32_t, 0x0063a194)[index]) * static_cast<int64_t>(scale);
            const uint32_t lower = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 6);
            flags = subFlags32(static_cast<uint32_t>(diff), lower);
            if (diff <= static_cast<int32_t>(lower)) {
                eax = 0xffffc000u;
                edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
            } else {
                uint32_t numerator = static_cast<uint32_t>(diff);
                if (static_cast<int32_t>(numerator) < 0) {
                    numerator = 0u - numerator;
                }
                uint32_t divisor = lower;
                if (static_cast<int32_t>(divisor) < 0) {
                    divisor = 0u - divisor;
                }
                const uint32_t high = ((static_cast<uint32_t>(static_cast<int32_t>(numerator) >> 31)) << 14) | (numerator >> 18);
                const uint32_t low = numerator << 14;
                int32_t remainder;
                int32_t quotient = gp4::idiv64(static_cast<int64_t>((static_cast<uint64_t>(high) << 32) | low), static_cast<int32_t>(divisor), &remainder);
                if (quotient < 0) {
                    quotient = 0;
                }
                if (quotient > 0x4000) {
                    quotient = 0x4000;
                }
                eax = 0u - static_cast<uint32_t>(quotient);
                flags = subFlags32(0u, static_cast<uint32_t>(quotient));
                edx = static_cast<uint32_t>(remainder);
            }
        }
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426947, FUN_00426947, "regs(eax:i32[-20000..20000], esi:ptr[0x100]:layout=rec_00426947) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_00426947+frac_00624790")
