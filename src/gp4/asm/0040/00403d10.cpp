#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t comparisonFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00403d10
void __cdecl FUN_00403d10(gp4::Regs* r) {
    const int32_t a = static_cast<int16_t>(r->eax);
    const int32_t b = static_cast<int16_t>(r->ecx);
    const int32_t c = static_cast<int16_t>(r->edx);

    // ratio = ((b - c) << 14) / (a - c), clamped to +-0x400000
    int32_t divisor = a - c;
    if (divisor == 0) divisor = 1;
    const int32_t dividend = static_cast<int32_t>(static_cast<uint32_t>(b - c) << 14);
    int32_t remainder;
    int32_t quotient = gp4::idiv64(static_cast<int64_t>(dividend), divisor, &remainder);

    uint32_t flags;
    if (quotient < 0) {
        uint32_t magnitude = 0u - static_cast<uint32_t>(quotient);
        if (magnitude >= 0x400000u) magnitude = 0x400000u;
        const uint32_t result = 0u - magnitude;
        // final neg: CF = source != 0, OF = source == 0x80000000
        flags = (magnitude != 0u ? 1u : 0u)
            | parityFlag8(result)
            | (((magnitude ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
            | (magnitude == 0x80000000u ? 0x800u : 0u);
        quotient = static_cast<int32_t>(result);
    } else {
        // flags stay those of cmp eax, 0x400000
        flags = comparisonFlags32(static_cast<uint32_t>(quotient), 0x400000u);
        if (static_cast<uint32_t>(quotient) >= 0x400000u) quotient = 0x400000;
    }

    r->eax = static_cast<uint32_t>(quotient);
    r->ecx = static_cast<uint32_t>(divisor);
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00403d10, FUN_00403d10, "regs(eax:u32, ecx:u32, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")
