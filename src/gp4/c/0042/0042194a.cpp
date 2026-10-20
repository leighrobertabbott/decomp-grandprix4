#include <gp4/gp4.h>

// Flags of `cmp left, right` (dword).
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul + "shrd eax, edx, 14": bits 14..45 of the signed 64-bit product
static inline uint32_t q14(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 14);
}

// low 32 bits of the signed product ("imul dword ptr [mem]" keeps them in eax)
static inline int32_t low32(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) * static_cast<uint32_t>(b));
}

// FUNCTION: GP4 0x0042194a
// Scales a set of limits by the factor at 0x00639630: three Q14 products (scale << 8) into
// 0x00608088/8c/90, and three clamped products into 0x00608010 (= 0x00608014, upper bound only),
// 0x00608080 and 0x00608084 (lower and upper bounds).
// Leaves eax = the last clamped value, edx = the high half of the last product and the flags of
// the last cmp (before the upper clamp).
void __cdecl FUN_0042194a(gp4::Regs* r) {
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00639630);
    const int32_t shifted = static_cast<int32_t>(static_cast<uint32_t>(scale) << 8);

    GP4_GLOBAL(uint32_t, 0x00608088) = q14(shifted, GP4_GLOBAL(int32_t, 0x00608048));
    GP4_GLOBAL(uint32_t, 0x0060808c) = q14(shifted, GP4_GLOBAL(int32_t, 0x0060804c));
    GP4_GLOBAL(uint32_t, 0x00608090) = q14(shifted, GP4_GLOBAL(int32_t, 0x00608050));

    int32_t value = low32(scale, GP4_GLOBAL(int32_t, 0x00608054));
    if (value > GP4_GLOBAL(int32_t, 0x00608058)) {
        value = GP4_GLOBAL(int32_t, 0x00608058);
    }
    GP4_GLOBAL(int32_t, 0x00608010) = value;
    GP4_GLOBAL(int32_t, 0x00608014) = value;

    value = low32(scale, GP4_GLOBAL(int32_t, 0x00608094));
    if (value < GP4_GLOBAL(int32_t, 0x00608098)) {
        value = GP4_GLOBAL(int32_t, 0x00608098);
    }
    if (value > GP4_GLOBAL(int32_t, 0x0060809c)) {
        value = GP4_GLOBAL(int32_t, 0x0060809c);
    }
    GP4_GLOBAL(int32_t, 0x00608080) = value;

    const int64_t product = static_cast<int64_t>(scale) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006080a4));
    value = static_cast<int32_t>(static_cast<uint32_t>(product));
    if (value < GP4_GLOBAL(int32_t, 0x006080a8)) {
        value = GP4_GLOBAL(int32_t, 0x006080a8);
    }
    const uint32_t flags = compareFlags32(static_cast<uint32_t>(value), GP4_GLOBAL(uint32_t, 0x006080ac));
    if (value > GP4_GLOBAL(int32_t, 0x006080ac)) {
        value = GP4_GLOBAL(int32_t, 0x006080ac);
    }
    GP4_GLOBAL(int32_t, 0x00608084) = value;

    r->eax = static_cast<uint32_t>(value);
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042194a, FUN_0042194a, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
