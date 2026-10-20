#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a logic operation (test) with a 32-bit result: CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit "neg value" (result = 0 - value).
static inline uint32_t negFlags(uint32_t value) {
    const uint32_t result = 0u - value;
    return parityFlag(result)
        | (value != 0u ? 1u : 0u)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

// The two interpolation routines return edx = interpolated word pair of the block at edi - 0x100.
// Separate helpers keep the two call sites distinct.
static __declspec(noinline) uint32_t lowBound(const gp4::Regs& base) {
    gp4::Regs c = base;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00403b19, c);
    return c.edx;
}
static __declspec(noinline) uint32_t highBound(const gp4::Regs& base) {
    gp4::Regs c = base;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00403b3f, c);
    return c.edx;
}

// Position `ecx` against the band around `eax` whose half-width is the interpolated bound
// (edi block, via 0x403b19 when ecx < eax, otherwise 0x403b3f) floored at [0x00622358]. Inside the
// band eax = ((ecx - edge) << 14) / (eax - edge) (signed, 1 when that is zero); outside it
// eax = -min(|ecx - edge| << 14 / [0x00622354], 0x80000). edx = 0 (ecx >= eax) or -1 (ecx < eax);
// ebp and ecx are saved and restored. The flags are those of the final `neg eax`, or of the
// `test ecx, ecx` before the idiv (which leaves them alone).
// FUNCTION: GP4 0x00403bb6
void __cdecl FUN_00403bb6(gp4::Regs* r) {
    const int32_t anchor = static_cast<int32_t>(r->eax);
    const int32_t position = static_cast<int32_t>(r->ecx);
    const int32_t floorValue = GP4_GLOBAL(int32_t, 0x00622358);

    uint32_t edge;
    uint32_t sideMarker;
    bool inside;
    if (position < anchor) {
        sideMarker = 0xffffffffu;
        edge = lowBound(*r) + static_cast<uint32_t>(anchor);
        if (!(static_cast<int32_t>(edge) >= floorValue)) {
            edge = static_cast<uint32_t>(floorValue);
        }
        edge = 0u - (edge - static_cast<uint32_t>(anchor));
        inside = position >= static_cast<int32_t>(edge);
    } else {
        sideMarker = 0u;
        edge = highBound(*r) - static_cast<uint32_t>(anchor);
        if (!(static_cast<int32_t>(edge) >= floorValue)) {
            edge = static_cast<uint32_t>(floorValue);
        }
        edge = edge + static_cast<uint32_t>(anchor);
        inside = position <= static_cast<int32_t>(edge);
    }

    uint32_t result;
    uint32_t flags;
    if (inside) {
        const uint32_t numerator = (static_cast<uint32_t>(position) - edge) << 14;
        uint32_t divisor = static_cast<uint32_t>(anchor) - edge;
        flags = logicFlags(divisor);
        if (divisor == 0u) {
            divisor = 1u;
        }
        int32_t remainder;
        result = static_cast<uint32_t>(gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(numerator)), static_cast<int32_t>(divisor), &remainder));
    } else {
        uint32_t distance = static_cast<uint32_t>(position) - edge;
        if (static_cast<int32_t>(distance) < 0) {
            distance = 0u - distance;
        }
        uint32_t remainder;
        uint32_t quotient = gp4::div64(static_cast<uint64_t>(distance) << 14, GP4_GLOBAL(uint32_t, 0x00622354), &remainder);
        if (static_cast<int32_t>(quotient) > 0x80000) {
            quotient = 0x80000u;
        }
        flags = negFlags(quotient);
        result = 0u - quotient;
    }

    r->eax = result;
    r->edx = sideMarker;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00403bb6, FUN_00403bb6, "regs(eax:i32[-0x800000..0x800000], ecx:i32[-0x800000..0x800000], edi:ptr[0x1ca]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=div_00403bb6")
