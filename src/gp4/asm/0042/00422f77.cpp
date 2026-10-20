#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Three Q-format 64-bit accumulators at 0x00608168 / 0x00608170 / 0x00608178 (low dword, high dword).
struct Unknown_00422f77_Accumulator {
    uint32_t Low;
    uint32_t High;
};

// (gain * (a - b)) >> 5 as a 64-bit arithmetic shift ("imul; shrd eax, edx, 5; sar edx, 5").
static inline int64_t ScaledProduct(int32_t gain, int32_t a, int32_t b) {
    const int32_t delta = static_cast<int32_t>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b));
    return (static_cast<int64_t>(gain) * delta) >> 5;
}

static inline uint64_t Load(const Unknown_00422f77_Accumulator* acc) {
    return (static_cast<uint64_t>(acc->High) << 32) | acc->Low;
}

static inline void Store(Unknown_00422f77_Accumulator* acc, uint64_t value) {
    acc->Low = static_cast<uint32_t>(value);
    acc->High = static_cast<uint32_t>(value >> 32);
}

// FUNCTION: GP4 0x00422f77
// Cross-products of the point at ebx (three i32) relative to the origin 0x6097b4/b8/bc with the
// three gains 0x623e18 / 0x623e1c / 0x623e20, each (gain * delta) >> 5 as a 64-bit value, added to /
// subtracted from the three 64-bit accumulators:
//   [0x608168] += g1c * dz - g20 * dy     [0x608170] += g20 * dx - g18 * dz     [0x608178] += g18 * dy - g1c * dx
// eax/edx hold the last product (low / high), the flags are those of the final adc.
void __cdecl FUN_00422f77(gp4::Regs* r) {
    const int32_t* point = reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(r->ebx));
    Unknown_00422f77_Accumulator* accA = reinterpret_cast<Unknown_00422f77_Accumulator*>(GP4_ARRAY(uint8_t, 0x00608168));
    Unknown_00422f77_Accumulator* accB = reinterpret_cast<Unknown_00422f77_Accumulator*>(GP4_ARRAY(uint8_t, 0x00608170));
    Unknown_00422f77_Accumulator* accC = reinterpret_cast<Unknown_00422f77_Accumulator*>(GP4_ARRAY(uint8_t, 0x00608178));

    const int32_t gain18 = GP4_GLOBAL(int32_t, 0x00623e18);
    const int32_t gain1c = GP4_GLOBAL(int32_t, 0x00623e1c);
    const int32_t gain20 = GP4_GLOBAL(int32_t, 0x00623e20);

    uint64_t a = Load(accA);
    a -= static_cast<uint64_t>(ScaledProduct(gain20, point[1], GP4_GLOBAL(int32_t, 0x006097b8)));
    a += static_cast<uint64_t>(ScaledProduct(gain1c, point[2], GP4_GLOBAL(int32_t, 0x006097bc)));
    Store(accA, a);

    uint64_t b = Load(accB);
    b -= static_cast<uint64_t>(ScaledProduct(gain18, point[2], GP4_GLOBAL(int32_t, 0x006097bc)));
    b += static_cast<uint64_t>(ScaledProduct(gain20, point[0], GP4_GLOBAL(int32_t, 0x006097b4)));
    Store(accB, b);

    uint64_t c = Load(accC);
    c -= static_cast<uint64_t>(ScaledProduct(gain1c, point[0], GP4_GLOBAL(int32_t, 0x006097b4)));
    const uint64_t last = static_cast<uint64_t>(ScaledProduct(gain18, point[1], GP4_GLOBAL(int32_t, 0x006097b8)));
    const uint32_t lastLow = static_cast<uint32_t>(last);
    const uint32_t lastHigh = static_cast<uint32_t>(last >> 32);

    // add low dword, then adc the high dword
    const uint32_t cLow = static_cast<uint32_t>(c);
    const uint32_t cHigh = static_cast<uint32_t>(c >> 32);
    const uint32_t newLow = cLow + lastLow;
    const uint32_t carryIn = newLow < cLow ? 1u : 0u;
    const uint32_t newHigh = cHigh + lastHigh + carryIn;
    accC->Low = newLow;
    accC->High = newHigh;

    const uint64_t wide = static_cast<uint64_t>(cHigh) + lastHigh + carryIn;
    const uint32_t flags = static_cast<uint32_t>(wide >> 32)
        | parityFlag8(newHigh)
        | (((cHigh ^ lastHigh ^ newHigh) & 0x10u) != 0u ? 0x10u : 0u)
        | (newHigh == 0u ? 0x40u : 0u)
        | ((newHigh & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((~(cHigh ^ lastHigh) & (cHigh ^ newHigh)) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = lastLow;
    r->edx = lastHigh;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00422f77, FUN_00422f77, "regs(ebx:ptr[0x10]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
