#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004159e3
void __cdecl FUN_004159e3(gp4::Regs* r) {
    int32_t factor = GP4_GLOBAL(int32_t, 0x0063b834) - GP4_GLOBAL(int32_t, 0x0066d814);
    if (factor < 0) {
        factor = 0;
    }

    const int64_t first = static_cast<int64_t>(factor) * GP4_GLOBAL(int32_t, 0x00603a14);
    int32_t value = static_cast<int32_t>(0u - static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14))
        + GP4_GLOBAL(int32_t, 0x00603a08);
    if (value < GP4_GLOBAL(int32_t, 0x00603a0c)) {
        value = GP4_GLOBAL(int32_t, 0x00603a0c);
    }
    GP4_GLOBAL(int32_t, 0x00603a00) = value;

    const int64_t second = static_cast<int64_t>(factor) * GP4_GLOBAL(int32_t, 0x00603a18);
    const uint32_t sum = (0u - static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14))
        + GP4_GLOBAL(uint32_t, 0x00603a10);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00603a5c);
    uint32_t result = sum;
    if (static_cast<int32_t>(sum) < static_cast<int32_t>(limit)) {
        result = limit;
    }
    GP4_GLOBAL(uint32_t, 0x00603a04) = result;

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags32(sum, limit);
}
GP4_IMPL(0x004159e3, FUN_004159e3, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
