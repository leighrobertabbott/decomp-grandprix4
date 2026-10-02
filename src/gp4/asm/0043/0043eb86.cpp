#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp left, right (neg v is cmp 0, v)
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Reciprocal-style scale: 0x1000000 / clamp(|x|, 0x240) with x's sign; 0 stays 0.
// The idiv leaves the flags of the preceding cmp/neg; edx and ebp are preserved.
// FUNCTION: GP4 0x0043eb86
void __cdecl FUN_0043eb86(gp4::Regs* r) {
    const uint32_t in = r->eax;
    uint32_t flags;
    uint32_t out = in;
    if (in == 0u) {
        flags = subFlags(in, 0u);
    } else {
        uint32_t d = in;
        if ((int32_t)d < 0) {
            d = 0u - d;
            if ((int32_t)d < 0x240) {
                d = 0x240u;
            }
            d = 0u - d;
            flags = subFlags(0u, 0u - d);
        } else {
            flags = subFlags(d, 0x240u);
            if ((int32_t)d < 0x240) {
                d = 0x240u;
            }
        }
        out = (uint32_t)(0x1000000 / (int32_t)d);
    }
    r->eax = out;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043eb86, FUN_0043eb86, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
