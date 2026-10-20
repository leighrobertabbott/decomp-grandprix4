#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp left, right".
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00413045
// eax = max((DAT_00639644 * DAT_0060345c) >> 14, DAT_00603460); edx holds the
// high half of the product and the flags are those of the final compare.
void __cdecl FUN_00413045(gp4::Regs* r) {
    const int32_t sample = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00639644)));
    const int32_t scale = GP4_GLOBAL(int32_t, 0x0060345c);
    const int64_t product = static_cast<int64_t>(sample) * static_cast<int64_t>(scale);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);

    uint32_t result = (low >> 14) | (high << 18);
    const uint32_t floorValue = GP4_GLOBAL(uint32_t, 0x00603460);
    const uint32_t flags = compareFlags32(result, floorValue);
    if (static_cast<int32_t>(result) < static_cast<int32_t>(floorValue)) {
        result = floorValue;
    }

    r->eax = result;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00413045, FUN_00413045, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
