#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right" (CF included)
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// flags of "xor reg, reg" (CF = OF = AF = 0)
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

struct AxisResult {
    uint32_t eax;
    uint32_t edx;       // eax ^ ecx
    bool signsDiffer;   // js after the xor
};

// One axis: ecx = a.pos - b.pos; eax = ecx + (((a.vel - b.vel) * [0x603854]) >> 16); when the
// signs of eax and ecx agree eax becomes min(|eax|, |ecx|) (signed compares, so 0x80000000
// survives the neg).
static inline AxisResult axis(uint8_t* a, uint8_t* b, uint32_t posOff, uint32_t velOff) {
    const uint32_t ecx = GP4_FIELD(uint32_t, a, posOff) - GP4_FIELD(uint32_t, b, posOff);
    const int32_t diff = static_cast<int32_t>(GP4_FIELD(uint32_t, a, velOff) - GP4_FIELD(uint32_t, b, velOff));
    const int64_t product = static_cast<int64_t>(diff) * GP4_GLOBAL(int32_t, 0x00603854);
    uint32_t eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16) + ecx;
    AxisResult out;
    out.edx = eax ^ ecx;
    out.signsDiffer = (out.edx & 0x80000000u) != 0u;
    if (!out.signsDiffer) {
        int32_t m = static_cast<int32_t>(ecx);
        int32_t e = static_cast<int32_t>(eax);
        if (m < 0) m = static_cast<int32_t>(0u - static_cast<uint32_t>(m));
        if (e < 0) e = static_cast<int32_t>(0u - static_cast<uint32_t>(e));
        if (e > m) e = m;
        eax = static_cast<uint32_t>(e);
    }
    out.eax = eax;
    return out;
}

// FUNCTION: GP4 0x0043aa07
// Overlap test of two records (edi, ebx) on three axes (position at +0/+8/+4, velocity at
// +0x18/+0x20/+0x1c, summed radius [+0x68] of both). CF clear as soon as one axis has a gap of at
// least the summed radius (the axis difference is limited by the velocity-corrected one when
// their signs agree); CF set when no axis does (an axis whose corrected difference changes sign
// is never a gap). eax/edx hold the last tested axis; ecx and ebp are saved and restored.
// Flags are those of the last cmp (or of the xor on the sign-differs exit), with CF set by stc/clc.
void __cdecl FUN_0043aa07(gp4::Regs* r) {
    uint8_t* const a = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* const b = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t limit = GP4_FIELD(uint32_t, a, 0x68) + GP4_FIELD(uint32_t, b, 0x68);

    static const uint32_t posOff[3] = {0u, 8u, 4u};
    static const uint32_t velOff[3] = {0x18u, 0x20u, 0x1cu};

    for (int i = 0; i < 3; ++i) {
        const AxisResult x = axis(a, b, posOff[i], velOff[i]);
        const bool gap = !x.signsDiffer && static_cast<int32_t>(x.eax) >= static_cast<int32_t>(limit);
        if (gap || i == 2) {
            r->eax = x.eax;
            r->edx = x.edx;
            if (x.signsDiffer) {
                r->eflags = (r->eflags & ~0x8d5u) | logicFlags(x.edx) | 1u;
            } else if (gap) {
                r->eflags = (r->eflags & ~0x8d5u) | (subFlags(x.eax, limit) & ~1u);
            } else {
                r->eflags = (r->eflags & ~0x8d5u) | subFlags(x.eax, limit) | 1u;
            }
            return;
        }
    }
}
GP4_IMPL(0x0043aa07, FUN_0043aa07, "regs(ebx:ptr[0x70]:bytes, edi:ptr[0x70]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
