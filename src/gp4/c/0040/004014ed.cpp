#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by a 32-bit OR: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x004014ed
// Draws two bytes from the 0x004014ba generator: eax = (first << 8) | second.
void __cdecl FUN_004014ed(gp4::Regs* r) {
    const uint32_t savedEdx = r->edx;

    gp4::call_regs(0x004014ba, *r);
    const uint32_t high = (r->eax & 0xffu) << 8;

    gp4::call_regs(0x004014ba, *r);
    const uint32_t result = (r->eax & 0xffu) | high;

    r->eax = result;
    r->edx = savedEdx;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(result);
}
GP4_IMPL(0x004014ed, FUN_004014ed, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0062d1b4")
