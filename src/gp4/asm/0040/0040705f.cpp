#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

static inline uint32_t resultFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// cmp left, right (32-bit).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | resultFlags32(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// add left, right (32-bit).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | resultFlags32(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a byte XOR: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint8_t value) {
    return parityFlag8(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// imul + shrd eax, edx, 0xe. Pinned oracle flags: CF = last bit out, AF = 0,
// OF = bit31(value shifted by 13 ^ value shifted by 14).
struct MulShift {
    uint32_t value;
    uint32_t high;
    uint32_t flags;
};
static inline MulShift mulShift14(int32_t a, int32_t b) {
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b));
    MulShift m;
    m.value = static_cast<uint32_t>(product >> 14);
    m.high = static_cast<uint32_t>(product >> 32);
    const uint32_t previous = static_cast<uint32_t>(product >> 13);
    m.flags = (previous & 1u)
        | resultFlags32(m.value)
        | (((previous ^ m.value) & 0x80000000u) != 0u ? 0x800u : 0u);
    return m;
}

// Interpolate between a word pair of this record (+lo) and the next record (+0x160+lo).
static inline uint32_t lerpWord(uint8_t* rec, int offset, int shift, int32_t t, uint32_t* edx) {
    const int32_t from = static_cast<int32_t>(GP4_FIELD(int16_t, rec, offset)) << shift;
    const int32_t to = static_cast<int32_t>(GP4_FIELD(int16_t, rec, offset + 0x160)) << shift;
    const MulShift m = mulShift14(static_cast<int32_t>(static_cast<uint32_t>(to) - static_cast<uint32_t>(from)), t);
    *edx = m.high;
    return m.value + static_cast<uint32_t>(from);
}

// Clamp num to 0..max(den,1) and return (num << 14) / den (idiv).
static inline uint32_t fraction14(uint32_t num, uint32_t den, uint32_t* ebp) {
    int32_t d = static_cast<int32_t>(den);
    if (d < 1) d = 1;
    int32_t n = static_cast<int32_t>(num);
    if (n < 0) n = 0;
    if (n > d) n = d;
    *ebp = static_cast<uint32_t>(d);
    int32_t remainder;
    return static_cast<uint32_t>(gp4::idiv64(static_cast<int64_t>(n) << 14, d, &remainder));
}

// FUNCTION: GP4 0x0040705f
void __cdecl FUN_0040705f(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;
    uint32_t eax = 0;
    uint32_t flags;

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    if (r->edi >= limit) {
        r->eax = 0;
        r->eflags = (r->eflags & ~0x8d5u) | subFlags32(r->edi, limit);
        return;
    }
    const uint8_t side = static_cast<uint8_t>(static_cast<uint8_t>(edx) ^ GP4_GLOBAL(uint8_t, 0x00868582));
    edx = (edx & 0xffffff00u) | side;
    if ((side & 0x80u) != 0u) {
        r->eax = 0;
        r->edx = edx;
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(side);
        return;
    }

    bool done = false;
    if ((GP4_FIELD(uint16_t, rec, 0x2b4) | GP4_FIELD(uint16_t, rec, 0x2b8)
         | GP4_FIELD(uint16_t, rec, 0x154) | GP4_FIELD(uint16_t, rec, 0x158)) != 0u) {
        GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(ext(GP4_GLOBAL(double, 0x00627958)) * ext(GP4_GLOBAL(double, 0x006f2b04)));
        const int32_t position = GP4_GLOBAL(int32_t, 0x006f2bf4);
        GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(ext(GP4_GLOBAL(double, 0x00627950)) * ext(GP4_GLOBAL(double, 0x006f2b04)));
        int32_t t = GP4_GLOBAL(int32_t, 0x006f2bf4);
        if ((GP4_GLOBAL(uint8_t, 0x00868582) & 0x80u) == 0u) t = 0x4000 - t;

        if (GP4_FIELD(uint16_t, rec, 0x2b4) != 0u && GP4_FIELD(uint16_t, rec, 0x154) != 0u) {
            GP4_GLOBAL(uint32_t, 0x006277b8) = lerpWord(rec, 0x156, 6, t, &edx);
            const uint32_t knot = lerpWord(rec, 0x154, 0, t, &edx);
            GP4_GLOBAL(uint32_t, 0x006277bc) = knot;
            if (position <= static_cast<int32_t>(knot)) {
                const uint32_t q = fraction14(static_cast<uint32_t>(position), knot, &ebp);
                const MulShift m = mulShift14(static_cast<int32_t>(q), GP4_GLOBAL(int32_t, 0x006277b8));
                eax = m.value;
                edx = m.high;
                flags = m.flags;
                done = true;
            }
        }

        if (!done && GP4_FIELD(uint16_t, rec, 0x2b8) != 0u && GP4_FIELD(uint16_t, rec, 0x158) != 0u) {
            GP4_GLOBAL(uint32_t, 0x006277c0) = lerpWord(rec, 0x15a, 6, t, &edx);
            const uint32_t knot = lerpWord(rec, 0x158, 0, t, &edx);
            GP4_GLOBAL(uint32_t, 0x006277c4) = knot;
            if (position < static_cast<int32_t>(knot)) {
                const uint32_t first = GP4_GLOBAL(uint32_t, 0x006277bc);
                int32_t q = static_cast<int32_t>(fraction14(static_cast<uint32_t>(position) - first,
                                                            GP4_GLOBAL(uint32_t, 0x006277c4) - first, &ebp));
                if (q < 0) q = 0;
                if (q > 0x4000) q = 0x4000;
                ebp = static_cast<uint32_t>(q);
                const uint32_t base = GP4_GLOBAL(uint32_t, 0x006277b8);
                const MulShift m = mulShift14(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006277c0) - base), q);
                edx = m.high;
                flags = addFlags32(m.value, base);
                eax = m.value + base;
            } else {
                const uint32_t q = fraction14(static_cast<uint32_t>(position) - knot, 0x4000u - knot, &ebp);
                const MulShift m = mulShift14(static_cast<int32_t>(0x4000u - q), GP4_GLOBAL(int32_t, 0x006277c0));
                eax = m.value;
                edx = m.high;
                flags = m.flags;
            }
            done = true;
        }
    }
    if (!done) {
        // The zero result of the last "or ax"/"test word" leaves ZF and PF set.
        eax = 0;
        flags = 0x44u;
    }

    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040705f, FUN_0040705f, "regs(edi:u32[8995744..8995744], edx:u32, ebp:u32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); cases=400; globals=layout=state_0040705f+frac_0040705f+zeros_0040705f")
