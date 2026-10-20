#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Random integer in 1..eax (1 when eax is 0): one byte from the generator 0x004014ba modulo the
// bound, plus one. edx and ebp are preserved. The flags are those of the final "inc eax"
// (CF stays 0 from "test ebp, ebp"; div leaves the flags alone in the oracle).
// FUNCTION: GP4 0x0040150b
void __cdecl FUN_0040150b(gp4::Regs* r) {
    uint32_t bound = r->eax;
    const uint32_t savedEdx = r->edx;

    gp4::call_regs(0x004014ba, *r);
    const uint32_t draw = r->eax & 0xffu;

    if (bound == 0u) {
        bound = 1u;
    }
    const uint32_t remainder = draw % bound;
    const uint32_t result = remainder + 1u;

    const uint32_t flags = parityFlag(result)
        | (((remainder ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (remainder == 0x7fffffffu ? 0x800u : 0u);

    r->eax = result;
    r->edx = savedEdx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040150b, FUN_0040150b, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0062d1b4")
