#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (result < left ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043ca01
void __cdecl FUN_0043ca01(gp4::Regs* r) {
    const uint32_t end = GP4_GLOBAL(uint32_t, 0x006227e0);
    const uint32_t start = GP4_GLOBAL(uint32_t, 0x006227dc);
    uint32_t result;
    uint32_t flags;
    if (GP4_GLOBAL(int32_t, 0x00622780) >= 0x10) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(end - start))
            * GP4_GLOBAL(int32_t, 0x006227a0);
        const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        result = shifted + start;
        flags = additionFlags32(shifted, start);
    } else if (static_cast<int32_t>(start) >= 0x180) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(end - start))
            * static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006227a0) >> 1);
        const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        result = shifted + start;
        flags = additionFlags32(shifted, start);
    } else {
        result = start;
        flags = subtractionFlags32(start, 0x180u);
    }
    r->eax = result;
    r->edx = start;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ca01, FUN_0043ca01, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
