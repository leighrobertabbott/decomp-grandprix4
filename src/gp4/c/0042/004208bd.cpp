#include <gp4/gp4.h>

static inline uint32_t SubtractFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (lhs < rhs ? 1u : 0u) | ((~parity & 1u) << 2)
        | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x004208bd
void __cdecl FUN_004208bd(gp4::Regs* r) {
    uint8_t* state = GP4_ARRAY(uint8_t, 0x006380d8);
    const int32_t divisor = GP4_GLOBAL(int32_t, 0x006228bc);
    const uint32_t firstDifference = GP4_FIELD(uint32_t, state, 0x2c8)
        - GP4_FIELD(uint32_t, state, 0x2c4);
    const int32_t firstQuotient = static_cast<int32_t>(firstDifference) / divisor;
    GP4_GLOBAL(int32_t, 0x0062244c) = firstQuotient;
    const uint32_t lhs = GP4_FIELD(uint32_t, state, 0x8c8);
    const uint32_t rhs = GP4_FIELD(uint32_t, state, 0x8c4);
    const uint32_t difference = lhs - rhs;
    const int32_t quotient = static_cast<int32_t>(difference) / divisor;
    const int32_t remainder = static_cast<int32_t>(difference) % divisor;
    GP4_GLOBAL(int32_t, 0x00622450) = quotient;
    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->ebx = 0x006380d8u;
    // IDIV arithmetic flags are undefined. The pinned Unicorn oracle
    // retains the flags from the second 32-bit subtraction.
    r->eflags = (r->eflags & ~0x8d5u) | SubtractFlags(lhs, rhs, difference);
}
GP4_IMPL(0x004208bd, FUN_004208bd, "regs() -> (eax,edx,ebx,cf,pf,af,zf,sf,of); globals=layout=state_004208bd")
