#include <gp4/gp4.h>

// Splits a weight between two bodies: q = (a.weight << 14) / (a.weight + b.weight) (Q14 fraction, the
// divisor replaced by 1 when the sum is zero), then stores (0x4000 - q) * [0x608018] >> 14 at
// [0x608020] and either 0 or -((0x4000 - q) * [0x60801c] >> 14) << 16 at [0x608024] (the second
// form unless a.flags6f bit 1 is set and the car record selected by a.byte6c lacks bit 2 at +0xcc).
// pushal/popal restore every register, so only memory and the flags of the final shift survive:
// SHRD by 14 on the first path, SHL by 16 (after NEG) on the second.
static inline uint32_t parity_flag(uint32_t value) {
    uint32_t b = value & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;   // PF set when the low byte has an even number of set bits
}

// FUNCTION: GP4 0x0043a785
void __cdecl FUN_0043a785(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->eax));
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));

    const int32_t weight = GP4_FIELD(int32_t, first, 0x48);
    int32_t divisor = static_cast<int32_t>(static_cast<uint32_t>(weight) + GP4_FIELD(uint32_t, second, 0x48));
    if (divisor == 0) divisor = 1;
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(weight) << 14, divisor, &remainder);

    const uint32_t complement = 0x4000u - static_cast<uint32_t>(quotient);
    const uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0) + GP4_FIELD(uint8_t, first, 0x6c) * 0x41c;

    uint32_t flags;
    if ((GP4_FIELD(uint8_t, first, 0x6f) & 2u) && !(GP4_FIELD(uint8_t, const_cast<uint8_t*>(car), 0xcc) & 4u)) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(complement)) * GP4_GLOBAL(int32_t, 0x00608018);
        const uint32_t low = static_cast<uint32_t>(product);
        const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);
        GP4_GLOBAL(uint32_t, 0x00608020) = result;
        GP4_GLOBAL(uint32_t, 0x00608024) = 0u;
        // SHRD by 14 (pinned oracle model): CF = bit 13 of the low dword, AF = 0, OF = bit31(previous ^ result).
        flags = ((low >> 13) & 1u) | parity_flag(result) | (result == 0 ? 0x40u : 0u)
              | ((result & 0x80000000u) ? 0x80u : 0u) | (((previous ^ result) >> 31) << 11);
    } else {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(complement)) * GP4_GLOBAL(int32_t, 0x00608018);
        GP4_GLOBAL(uint32_t, 0x00608020) = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        const int64_t product2 = static_cast<int64_t>(static_cast<int32_t>(complement)) * GP4_GLOBAL(int32_t, 0x0060801c);
        const uint32_t negated = 0u - static_cast<uint32_t>(static_cast<uint64_t>(product2) >> 14);
        const uint32_t result = negated << 16;
        GP4_GLOBAL(uint32_t, 0x00608024) = result;
        // SHL by 16: CF = bit 16 of the operand, AF = 0, OF = bit31((operand << 15) ^ result).
        flags = ((negated >> 16) & 1u) | parity_flag(result) | (result == 0 ? 0x40u : 0u)
              | ((result & 0x80000000u) ? 0x80u : 0u) | ((((negated << 15) ^ result) >> 31) << 11);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043a785, FUN_0043a785, "regs(eax:ptr[0x78]:layout=obj_0043a785, edx:ptr[0x78]:layout=obj_0043a785) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carcc_0043a785")
