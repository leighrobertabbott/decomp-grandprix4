#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp left, right" (32-bit).
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Adds eax to the signed word at esi+0xb6, clamps the sum to [DAT_006223f4, 0x7fff] (upper clamp
// first, then the lower bound wins) and stores the low word back. eax = the clamped value; edx is
// saved and restored; the flags are those of the final `cmp eax, [0x6223f4]`.
// FUNCTION: GP4 0x0042a0cf
void __cdecl FUN_0042a0cf(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    int32_t value = static_cast<int32_t>(r->eax) + static_cast<int32_t>(GP4_FIELD(int16_t, record, 0xb6));
    if (value > 0x7fff) {
        value = 0x7fff;
    }
    const int32_t floorValue = GP4_GLOBAL(int32_t, 0x006223f4);
    const uint32_t flags = compareFlags32(static_cast<uint32_t>(value), static_cast<uint32_t>(floorValue));
    if (value < floorValue) {
        value = floorValue;
    }
    GP4_FIELD(int16_t, record, 0xb6) = static_cast<int16_t>(value);

    r->eax = static_cast<uint32_t>(value);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042a0cf, FUN_0042a0cf, "regs(eax:i32, esi:ptr[0xbe]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
