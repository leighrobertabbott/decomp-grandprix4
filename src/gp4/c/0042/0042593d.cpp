#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0042593d
// Two draws from the 0x00401462 generator: eax = ((second << 8 | first) + 1) >> 2.
void __cdecl FUN_0042593d(gp4::Regs* r) {
    const uint32_t savedEdx = r->edx;

    gp4::call_regs(0x00401462, *r);
    const uint32_t first = r->eax;

    gp4::call_regs(0x00401462, *r);
    const uint32_t sum = (((r->eax & 0xffu) << 8) | (first & 0xffu)) + 1u;
    const uint32_t result = sum >> 2;

    // Flags of "shr eax, 2": CF = last bit shifted out, AF = 0,
    // OF = bit31(sum >> 1) ^ bit31(result).
    const uint32_t previous = sum >> 1;
    const uint32_t flags = (previous & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->edx = savedEdx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042593d, FUN_0042593d, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
