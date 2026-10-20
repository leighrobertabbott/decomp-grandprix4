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

// Flags of "cmp lhs, rhs" with CF forced to `carry` (stc / clc after the compare).
static inline uint32_t CompareFlags(uint32_t lhs, uint32_t rhs, uint32_t carry) {
    return (SubtractFlags(lhs, rhs, lhs - rhs) & ~1u) | carry;
}

// FUNCTION: GP4 0x0043c54b
void __cdecl FUN_0043c54b(gp4::Regs* r) {
    uint8_t* self = reinterpret_cast<uint8_t*>(r->esi);
    const int32_t input = static_cast<int32_t>(r->eax);

    GP4_GLOBAL(uint8_t, 0x006227f8) = 0;

    // (global / 60) * multiplier >> 14, at least 1.
    const uint32_t quotient = GP4_GLOBAL(uint32_t, 0x00622778) / 60u;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(quotient))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006227a0));
    int32_t divisor = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
    if (divisor < 1) divisor = 1;

    gp4::Regs callee{};
    gp4::call_regs(0x0043c4c1, callee);
    int32_t value = static_cast<int32_t>(callee.eax) - static_cast<int32_t>(GP4_FIELD(uint8_t, self, 0x233));
    if (value < 0) value = 0;
    int32_t rem;
    int32_t ratio = gp4::idiv64(static_cast<int64_t>(static_cast<uint32_t>(value) << 14), divisor, &rem);
    if (ratio < 0) ratio = 0;
    if (ratio > 0x4000) ratio = 0x4000;

    const int32_t level = static_cast<int32_t>(GP4_FIELD(uint8_t, self, 0x7f));
    uint32_t lhs, rhs, carry;
    if (level < 2) {
        lhs = static_cast<uint32_t>(input);
        rhs = 2;
        carry = 1;
        if (input >= 2) GP4_GLOBAL(uint8_t, 0x006227f8) = 0xff;
    } else if (input < 2) {
        lhs = static_cast<uint32_t>(ratio);
        rhs = 0x2000;
        carry = ratio >= 0x2000 ? 1u : 0u;
    } else {
        uint32_t diff = static_cast<uint32_t>(input) - static_cast<uint32_t>(level);
        if (static_cast<int32_t>(diff) < 0) diff = 0u - diff;
        if (diff == 2) {
            lhs = static_cast<uint32_t>(ratio);
            rhs = 0x2000;
            carry = ratio >= 0x2000 ? 1u : 0u;
        } else if (static_cast<int32_t>(diff) > 2) {
            lhs = diff;
            rhs = 2;
            carry = 1;
            GP4_GLOBAL(uint8_t, 0x006227f8) = 0xff;
        } else {
            lhs = static_cast<uint32_t>(ratio);
            rhs = 0x4000;
            carry = ratio >= 0x4000 ? 1u : 0u;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | CompareFlags(lhs, rhs, carry);
}
GP4_IMPL(0x0043c54b, FUN_0043c54b, "regs(eax:i32, esi:ptr[0x240]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
