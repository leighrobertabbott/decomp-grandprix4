#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004080de
void __cdecl FUN_004080de(gp4::Regs* r) {
    const uint32_t left = GP4_GLOBAL(uint8_t, 0x00622448);
    const uint32_t right = 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    // cmp byte [0x622448], 0xff, then stc (equal) or clc (not equal) rewrites CF
    const uint32_t flags = parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u)
        | (left == right ? 1u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004080de, FUN_004080de, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
