#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `shrd low, high, n` (1 < n < 32), the pinned oracle model: CF = last bit shifted out,
// AF = 0, OF = bit31(previous ^ result) with previous the 64-bit value shifted by n - 1.
static inline uint32_t shrdFlags(uint32_t low, uint32_t high, uint32_t n) {
    const uint64_t wide = (static_cast<uint64_t>(high) << 32) | low;
    const uint32_t result = static_cast<uint32_t>(wide >> n);
    const uint32_t previous = static_cast<uint32_t>(wide >> (n - 1u));
    return ((previous & 1u) != 0u ? 1u : 0u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Clamps the signed 64-bit edx:eax to [-limit, limit] with 0x00423883 (eax out, ebp in).
static inline int32_t clampPair(gp4::Regs* r, uint32_t low, uint32_t high, uint32_t limit) {
    gp4::Regs callee = *r;
    callee.eax = low;
    callee.edx = high;
    callee.ebp = limit;
    gp4::call_regs(0x00423883, callee);
    return static_cast<int32_t>(callee.eax);
}

// FUNCTION: GP4 0x004238ac
// ratio = (min(0x00608164, limit) << 14) / limit, with limit = 0x0060384c (1 when zero) in Q14.
// The six dword pairs at 0x00607ad0.. are each clamped (0x00423883) to +-0x00607ac8 (the first
// three) or +-0x00607acc (the last three), multiplied by the ratio and stored shifted right by 9
// (0x00607b00/04/08) or by 14 (0x00607b0c/10/14). Leaves eax = the last stored value, edx = the
// high half of the last product, ebp = 0x00607acc and the flags of the last shrd.
void __cdecl FUN_004238ac(gp4::Regs* r) {
    int32_t value = GP4_GLOBAL(int32_t, 0x00608164);
    int32_t limit = GP4_GLOBAL(int32_t, 0x0060384c);
    if (limit == 0) {
        limit = 1;
    }
    if (value > limit) {
        value = limit;
    }
    const int64_t scaled = static_cast<int64_t>(static_cast<uint64_t>(static_cast<int64_t>(value)) << 14);
    int32_t rem;
    const int32_t ratio = gp4::idiv64(scaled, limit, &rem);

    const uint32_t bigLimit = GP4_GLOBAL(uint32_t, 0x00607ac8);
    int64_t product;
    int32_t clamped;

    clamped = clampPair(r, GP4_GLOBAL(uint32_t, 0x00607ad0), GP4_GLOBAL(uint32_t, 0x00607ad4), bigLimit);
    product = static_cast<int64_t>(clamped) * static_cast<int64_t>(ratio);
    GP4_GLOBAL(uint32_t, 0x00607b00) = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 9);

    clamped = clampPair(r, GP4_GLOBAL(uint32_t, 0x00607ad8), GP4_GLOBAL(uint32_t, 0x00607adc), bigLimit);
    product = static_cast<int64_t>(clamped) * static_cast<int64_t>(ratio);
    GP4_GLOBAL(uint32_t, 0x00607b04) = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 9);

    clamped = clampPair(r, GP4_GLOBAL(uint32_t, 0x00607ae0), GP4_GLOBAL(uint32_t, 0x00607ae4), bigLimit);
    product = static_cast<int64_t>(clamped) * static_cast<int64_t>(ratio);
    GP4_GLOBAL(uint32_t, 0x00607b08) = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 9);

    const uint32_t smallLimit = GP4_GLOBAL(uint32_t, 0x00607acc);

    clamped = clampPair(r, GP4_GLOBAL(uint32_t, 0x00607ae8), GP4_GLOBAL(uint32_t, 0x00607aec), smallLimit);
    product = static_cast<int64_t>(clamped) * static_cast<int64_t>(ratio);
    GP4_GLOBAL(uint32_t, 0x00607b0c) = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    clamped = clampPair(r, GP4_GLOBAL(uint32_t, 0x00607af0), GP4_GLOBAL(uint32_t, 0x00607af4), smallLimit);
    product = static_cast<int64_t>(clamped) * static_cast<int64_t>(ratio);
    GP4_GLOBAL(uint32_t, 0x00607b10) = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    clamped = clampPair(r, GP4_GLOBAL(uint32_t, 0x00607af8), GP4_GLOBAL(uint32_t, 0x00607afc), smallLimit);
    product = static_cast<int64_t>(clamped) * static_cast<int64_t>(ratio);
    const uint32_t out = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    GP4_GLOBAL(uint32_t, 0x00607b14) = out;

    r->eax = out;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->ebp = smallLimit;
    r->eflags = (r->eflags & ~0x8d5u)
        | shrdFlags(static_cast<uint32_t>(product), static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32), 14u);
}
GP4_IMPL(0x004238ac, FUN_004238ac, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_004238ac")
