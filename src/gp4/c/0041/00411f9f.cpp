#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// value * 0x145f0 >> 8 (64-bit intermediate) divided by the divisor: mul, shrd/shr, one DIV.
static inline uint32_t scaledQuotient(uint32_t value, uint32_t divisor, uint32_t* remainder) {
    const uint64_t product = static_cast<uint64_t>(value) * 0x145f0u;
    return gp4::div64(product >> 8, divisor, remainder);
}

// FUNCTION: GP4 0x00411f9f
void __cdecl FUN_00411f9f(gp4::Regs* r) {
    uint32_t remainder = 0;
    uint32_t quotient = 0;

    GP4_GLOBAL(uint32_t, 0x00638d6c) = scaledQuotient(GP4_GLOBAL(uint32_t, 0x0060134c), GP4_GLOBAL(uint32_t, 0x00601380), &remainder);
    GP4_GLOBAL(uint32_t, 0x00638d70) = scaledQuotient(GP4_GLOBAL(uint32_t, 0x00601350), GP4_GLOBAL(uint32_t, 0x00601380), &remainder);
    GP4_GLOBAL(uint32_t, 0x00638d74) = scaledQuotient(GP4_GLOBAL(uint32_t, 0x0060134c), GP4_GLOBAL(uint32_t, 0x0060137c), &remainder);
    GP4_GLOBAL(uint32_t, 0x00638d78) = scaledQuotient(GP4_GLOBAL(uint32_t, 0x00601350), GP4_GLOBAL(uint32_t, 0x0060137c), &remainder);
    GP4_GLOBAL(uint32_t, 0x00638d7c) = scaledQuotient(GP4_GLOBAL(uint32_t, 0x0060133c), GP4_GLOBAL(uint32_t, 0x00601384), &remainder);
    GP4_GLOBAL(uint32_t, 0x00638d80) = scaledQuotient(GP4_GLOBAL(uint32_t, 0x00601344), GP4_GLOBAL(uint32_t, 0x00601384), &remainder);
    GP4_GLOBAL(uint32_t, 0x00638d84) = scaledQuotient(GP4_GLOBAL(uint32_t, 0x00601348), GP4_GLOBAL(uint32_t, 0x00601380), &remainder);
    GP4_GLOBAL(uint32_t, 0x00638d90) = scaledQuotient(GP4_GLOBAL(uint32_t, 0x00601358), GP4_GLOBAL(uint32_t, 0x00601380), &remainder);
    GP4_GLOBAL(uint32_t, 0x00638d94) = scaledQuotient(GP4_GLOBAL(uint32_t, 0x00601358), GP4_GLOBAL(uint32_t, 0x00601384), &remainder);
    GP4_GLOBAL(uint32_t, 0x00638d88) = scaledQuotient(GP4_GLOBAL(uint32_t, 0x0060136c), GP4_GLOBAL(uint32_t, 0x00601384), &remainder);
    quotient = scaledQuotient(GP4_GLOBAL(uint32_t, 0x0060136c), GP4_GLOBAL(uint32_t, 0x0060137c), &remainder);
    GP4_GLOBAL(uint32_t, 0x00638d8c) = quotient;

    // DIV leaves the flags of the preceding "shr edx, 8" (high half of the last product):
    // CF = bit 7 of the high dword, AF = OF = SF = 0, PF/ZF from the shifted value.
    const uint32_t high = static_cast<uint32_t>((static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x0060136c)) * 0x145f0u) >> 32);
    const uint32_t shifted = high >> 8;
    const uint32_t flags = ((high >> 7) & 1u) | parityFlag(shifted) | (shifted == 0u ? 0x40u : 0u);

    r->eax = quotient;
    r->edx = remainder;
    r->ebx = 0x00638058u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00411f9f, FUN_00411f9f, "regs() -> (eax, edx, ebx, cf, pf, af, zf, sf, of); globals=layout=state_00411f9f")
