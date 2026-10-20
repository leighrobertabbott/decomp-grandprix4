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

// One axis: ecx = a.pos - b.pos; eax = ecx - (((a.vel >> 8) - (b.vel >> 8)) * [0x603854] >> 16);
// when the signs of eax and ecx agree eax becomes min(|eax|, |ecx|) (signed compares, so
// 0x80000000 survives the neg).
static inline AxisResult axis(const uint8_t* a, const uint8_t* b, uint32_t posOff, uint32_t velOff) {
    const uint32_t ecx = GP4_FIELD(uint32_t, const_cast<uint8_t*>(a), posOff)
                       - GP4_FIELD(uint32_t, const_cast<uint8_t*>(b), posOff);
    const int32_t va = GP4_FIELD(int32_t, const_cast<uint8_t*>(a), velOff) >> 8;
    const int32_t vb = GP4_FIELD(int32_t, const_cast<uint8_t*>(b), velOff) >> 8;
    const int32_t diff = static_cast<int32_t>(static_cast<uint32_t>(va) - static_cast<uint32_t>(vb));
    const int64_t product = static_cast<int64_t>(diff) * GP4_GLOBAL(int32_t, 0x00603854);
    // shrd eax, edx, 16; neg eax; add eax, ecx
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16);
    uint32_t eax = ecx - shifted;
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

// FUNCTION: GP4 0x0043a964
// Compares two records (esi, ebx) on two axes (x at +0, y at +4; velocities at +0xa4/+0xa8).
// CF clear when the first axis (x) shows a gap of at least 0x42666 (nothing more is tested),
// or the second axis (y) does; CF set when both axes are close (or the signs of the second
// axis' difference and its velocity-corrected value differ). eax/edx hold the last axis'
// values; ecx and ebp are preserved (push/pop). Flags are those of the last cmp (or the
// xor on the sign-differs exit), with CF replaced by clc/stc.
void __cdecl FUN_0043a964(gp4::Regs* r) {
    const uint8_t* const from = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* const to = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t limit = 0x42666u;

    AxisResult x = axis(from, to, 0u, 0xa4u);
    if (!x.signsDiffer && static_cast<int32_t>(x.eax) >= static_cast<int32_t>(limit)) {
        r->eax = x.eax;
        r->edx = x.edx;
        r->eflags = (r->eflags & ~0x8d5u) | (subFlags(x.eax, limit) & ~1u);
        return;
    }

    AxisResult y = axis(from, to, 4u, 0xa8u);
    r->eax = y.eax;
    r->edx = y.edx;
    if (y.signsDiffer) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags(y.edx) | 1u;
    } else if (static_cast<int32_t>(y.eax) >= static_cast<int32_t>(limit)) {
        r->eflags = (r->eflags & ~0x8d5u) | (subFlags(y.eax, limit) & ~1u);
    } else {
        r->eflags = (r->eflags & ~0x8d5u) | subFlags(y.eax, limit) | 1u;
    }
}
GP4_IMPL(0x0043a964, FUN_0043a964, "regs(ebx:ptr[0xb0]:bytes, esi:ptr[0xb0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
