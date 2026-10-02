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

// FUNCTION: GP4 0x0040c4c3
void __cdecl FUN_0040c4c3(gp4::Regs* r) {
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x0062aca4) << 23;
    const uint32_t maskX = (GP4_GLOBAL(uint32_t, 0x0062aca4) << 24) - 1u;
    const uint32_t maskY = (GP4_GLOBAL(uint32_t, 0x0062aca8) << 24) - 1u;
    const uint32_t x = (base + GP4_GLOBAL(uint32_t, 0x0063b828)) & maskX;
    const uint32_t y = (base + GP4_GLOBAL(uint32_t, 0x0063b82c)) & maskY;
    const uint32_t shiftX = (GP4_GLOBAL(uint32_t, 0x0062acac) - 1u) & 31u;
    const uint32_t shiftY = (GP4_GLOBAL(uint32_t, 0x0062acb0) - 1u) & 31u;
    const uint32_t shiftedX = static_cast<uint32_t>(static_cast<int32_t>(x) >> shiftX);
    const uint32_t shiftedY = static_cast<uint32_t>(static_cast<int32_t>(y) >> shiftY);
    r->eax = shiftedX - 0x1000000u;
    r->edx = shiftedY - 0x1000000u;
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags32(shiftedY, 0x1000000u);
}
GP4_IMPL(0x0040c4c3, FUN_0040c4c3, "regs() -> (eax, edx, cf, pf, af, zf, sf, of)")
