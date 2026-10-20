#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// pushal wrapper: when the byte at 0x007ab020 equals 2, 0x0044557c resets the renderer state. popal
// restores every register but not the flags (those of `cmp byte [0x7ab020], 2`, or of 0x0044557c).
// FUNCTION: GP4 0x004085f9
void __cdecl FUN_004085f9(gp4::Regs* r) {
    const uint32_t left = GP4_GLOBAL(uint8_t, 0x007ab020);
    const uint32_t result = (left - 2u) & 0xffu;
    uint32_t flags = (left < 2u ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ 2u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ 2u) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
    if (left == 2u) {
        gp4::Regs call = *r;
        gp4::call_regs(0x0044557c, call);
        flags = call.eflags & 0x8d5u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004085f9, FUN_004085f9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
