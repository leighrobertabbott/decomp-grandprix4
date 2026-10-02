#include <gp4/gp4.h>

static inline uint32_t AddFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return static_cast<uint32_t>((static_cast<uint64_t>(lhs) + rhs) >> 32)
        | ((~parity & 1u) << 2) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x0043b920
void __cdecl FUN_0043b920(gp4::Regs* r) {
    const uint8_t selector = GP4_GLOBAL(uint8_t, 0x007aafd4);
    uint32_t offset = selector == 0u ? 0x41cu : 0x838u;
    uint32_t flags = 0u;
    for (uint32_t remaining = 22u; remaining != 0u; --remaining) {
        const uint8_t value = GP4_ARRAY(uint8_t, 0x0066fef0)[offset + 0x7au];
        if ((value & 0x80u) != 0u) {
            r->esi = 0x0066fef0u + offset;
            r->eflags = (r->eflags & ~0x8d5u) | 0x80u;
            return;
        }
        const uint32_t previous = 0x0066fef0u + offset;
        offset += 0x41cu;
        flags = AddFlags(previous, 0x41cu, 0x0066fef0u + offset);
    }
    r->esi = 0x0066fef0u + offset;
    r->eflags = (r->eflags & ~0x8d5u) | flags | 1u;
}
GP4_IMPL(0x0043b920, FUN_0043b920, "regs() -> (esi,cf,pf,af,zf,sf,of); globals=layout=state_0043b920")
