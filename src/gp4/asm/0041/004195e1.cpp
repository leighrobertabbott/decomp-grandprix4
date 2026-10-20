#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit "add left, right".
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

struct Axis {
    uint32_t ecx;        // scale + a
    uint32_t ebp;        // (a * scale) >> 10, low 32 bits
    int32_t remainder;   // idiv remainder
    uint32_t result;     // c - ((b - origin) * (scale + a)) / ebp
    uint32_t flags;      // flags of the final "add"
};

// result = c - ((b - origin) * (scale + a)) / (((int64)a * scale) >> 10)
static inline Axis solveAxis(int32_t a, int32_t b, int32_t origin, int32_t c, int32_t scale) {
    Axis o;
    o.ecx = static_cast<uint32_t>(scale) + static_cast<uint32_t>(a);
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(scale);
    o.ebp = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 10);
    const int32_t diff = static_cast<int32_t>(static_cast<uint32_t>(b) - static_cast<uint32_t>(origin));
    const int64_t numerator = static_cast<int64_t>(diff) * static_cast<int64_t>(static_cast<int32_t>(o.ecx));
    const int32_t quotient = gp4::idiv64(numerator, static_cast<int32_t>(o.ebp), &o.remainder);
    const uint32_t negated = 0u - static_cast<uint32_t>(quotient);
    o.result = negated + static_cast<uint32_t>(c);
    o.flags = addFlags(negated, static_cast<uint32_t>(c));
    return o;
}

// Computes the two projected positions at +0x2fc/+0x300 and +0x304/+0x308 of the esi block from
// the fields at +0x2cc/+0x2dc/+0x2ec and +0x2d4/+0x2e4/+0x2f4 and the scale [0x00601dac]
// with origins [0x00601d7c] and [0x00601d84]. Leaves the second axis in ecx/ebp/edx/eax.
// FUNCTION: GP4 0x004195e1
void __cdecl FUN_004195e1(gp4::Regs* r) {
    uint8_t* block = reinterpret_cast<uint8_t*>(r->esi);

    const Axis x = solveAxis(GP4_FIELD(int32_t, block, 0x2cc), GP4_FIELD(int32_t, block, 0x2ec),
                             GP4_GLOBAL(int32_t, 0x00601d7c), GP4_FIELD(int32_t, block, 0x2dc),
                             GP4_GLOBAL(int32_t, 0x00601dac));
    GP4_FIELD(uint32_t, block, 0x2fc) = x.result;
    GP4_FIELD(uint32_t, block, 0x300) = x.result;

    const Axis y = solveAxis(GP4_FIELD(int32_t, block, 0x2d4), GP4_FIELD(int32_t, block, 0x2f4),
                             GP4_GLOBAL(int32_t, 0x00601d84), GP4_FIELD(int32_t, block, 0x2e4),
                             GP4_GLOBAL(int32_t, 0x00601dac));
    GP4_FIELD(uint32_t, block, 0x304) = y.result;
    GP4_FIELD(uint32_t, block, 0x308) = y.result;

    r->eax = y.result;
    r->ecx = y.ecx;
    r->edx = static_cast<uint32_t>(y.remainder);
    r->ebp = y.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | y.flags;
}
GP4_IMPL(0x004195e1, FUN_004195e1, "regs(esi:ptr[0x310]:layout=rec_004195e1) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_004195e1")
