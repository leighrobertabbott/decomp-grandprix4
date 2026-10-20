#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of a 32-bit logic result (`or r, r`, `xor`, `test`): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `sub left, right` / `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Flags of a 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// Flags of a 32-bit `neg` of `value`.
static inline uint32_t negFlags32(uint32_t value) {
    return subFlags32(0u, value);
}

// imul + shrd eax, edx, 14: low dword of (a * b) >> 14.
static inline uint32_t mulQ14(uint32_t a, uint32_t b) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x0041befb
// Lateral-offset limiter. ebp = (dword +0x31c + +0x340) - +0x1c - +0x40 of the body in ebx. When it
// is nonzero the limit [0x00623b8c] (negated for a negative ebp) is turned into a Q14 blend
// weight: (|ebp| << 16) / |((limit*-2 + (+0x55c - +0x25c)) << 5) * [0x006013d4] / [0x00623af4]|
// (only when the two have opposite signs, the quotient fits and is below [0x0060384c]);
// otherwise [0x00623b98] = 0x4000 and the result is the signed limit. With a weight the result
// is half of (+0x55c - +0x25c), clamped to +-[0x00623b8c], blended with the limit by
// weight/[0x0060384c]. The result is stored in [0x00623b90] and returned in eax; [0x00623b98]
// receives the weight. edi and esi are preserved. Flags are those of the last flag writer.
void __cdecl FUN_0041befb(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint32_t eax;
    uint32_t ecx = 0;
    uint32_t edx = r->edx;
    uint32_t ebp;
    uint32_t edi = 0;
    uint32_t flags;

    GP4_GLOBAL(uint32_t, 0x00623b98) = 0;

    ebp = GP4_FIELD(uint32_t, body, 0x31c) + GP4_FIELD(uint32_t, body, 0x340);
    ebp -= GP4_FIELD(uint32_t, body, 0x1c);
    {
        const uint32_t last = GP4_FIELD(uint32_t, body, 0x40);
        flags = subFlags32(ebp, last);
        ebp -= last;
    }
    eax = 0;

    if (ebp != 0u) {
        edi = GP4_GLOBAL(uint32_t, 0x00623b8c);
        if (static_cast<int32_t>(ebp) < 0) {
            edi = 0u - edi;
        }
        eax = edi << 1;
        eax = 0u - eax;
        eax += GP4_FIELD(uint32_t, body, 0x55c);
        eax -= GP4_FIELD(uint32_t, body, 0x25c);
        eax <<= 5;

        int32_t remainder;
        const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(eax))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006013d4));
        const int32_t quotient = gp4::idiv64(scaled, GP4_GLOBAL(int32_t, 0x00623af4), &remainder);
        eax = static_cast<uint32_t>(quotient);
        edx = static_cast<uint32_t>(remainder);
        flags = logicFlags32(eax);
        if (eax != 0u) {
            const uint32_t sum = ebp;
            ebp = eax;
            eax = sum;
            edx = eax ^ ebp;
            flags = logicFlags32(edx);
            if (static_cast<int32_t>(edx) < 0) {
                if (static_cast<int32_t>(eax) < 0) {
                    eax = 0u - eax;
                }
                if (static_cast<int32_t>(ebp) < 0) {
                    ebp = 0u - ebp;
                }
                edx = static_cast<uint32_t>(static_cast<int32_t>(eax) >> 31);
                edx = (edx << 16) | (eax >> 16);
                eax <<= 16;
                flags = subFlags32(ebp, edx);
                if (ebp > edx) {
                    uint32_t remainder2;
                    eax = gp4::div64((static_cast<uint64_t>(edx) << 32) | eax, ebp, &remainder2);
                    edx = remainder2;
                    ecx = eax;
                    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x0060384c);
                    flags = subFlags32(ecx, limit);
                    if (ecx < limit) {
                        goto weighted;
                    }
                }
            }
        }
        GP4_GLOBAL(uint32_t, 0x00623b98) = 0x4000;
        eax = edi;
        goto done;
    }

weighted:
    eax = GP4_FIELD(uint32_t, body, 0x55c);
    eax -= GP4_FIELD(uint32_t, body, 0x25c);
    eax = static_cast<uint32_t>(static_cast<int32_t>(eax) >> 1);
    {
        const uint32_t cap = GP4_GLOBAL(uint32_t, 0x00623b8c);
        if (static_cast<int32_t>(eax) < 0) {
            eax = 0u - eax;
            if (!(eax < cap)) {
                eax = cap;
            }
            flags = negFlags32(eax);
            eax = 0u - eax;
        } else {
            flags = subFlags32(eax, cap);
            if (!(eax < cap)) {
                eax = cap;
            }
        }
    }
    flags = subFlags32(ecx, 0u);
    if (ecx != 0u) {
        const uint32_t held = eax;
        int32_t remainder;
        const int64_t numerator = static_cast<int64_t>(static_cast<int32_t>(ecx)) << 14;
        eax = static_cast<uint32_t>(gp4::idiv64(numerator, GP4_GLOBAL(int32_t, 0x0060384c), &remainder));
        edx = static_cast<uint32_t>(remainder);
        GP4_GLOBAL(uint32_t, 0x00623b98) = eax;
        ecx = eax;
        const uint32_t fromEdi = mulQ14(eax, edi);
        const int64_t rest = static_cast<int64_t>(static_cast<int32_t>(0x4000u - ecx))
            * static_cast<int64_t>(static_cast<int32_t>(held));
        edx = fromEdi;
        eax = static_cast<uint32_t>(static_cast<uint64_t>(rest) >> 14);
        flags = addFlags32(eax, edx);
        eax += edx;
    }

done:
    GP4_GLOBAL(uint32_t, 0x00623b90) = eax;
    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041befb, FUN_0041befb, "regs(edx:u32, ebx:rec[0x6380d8,0x300,0..0]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0041befb")
