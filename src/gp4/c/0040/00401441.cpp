#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? 0x4u : 0u;
}

// FUNCTION: GP4 0x00401441
// Draws two bytes from the 0x00401462 generator and returns ((first << 8) | second) >> 2.
// edx is saved/restored around the pair; the flags are those of the final `shr eax, 2`.
void __cdecl FUN_00401441(gp4::Regs* r) {
    gp4::Regs call = *r;

    gp4::call_regs(0x00401462, call);
    const uint32_t high = (call.eax & 0xffu) << 8;
    call.eax = high;

    gp4::call_regs(0x00401462, call);
    const uint32_t combined = (call.eax & 0xffu) | high;
    const uint32_t result = combined >> 2;

    // shr by 2: CF = last bit shifted out, AF = 0, OF = bit31(combined >> 1) ^ bit31(result) = 0
    const uint32_t flags = ((combined >> 1) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00401441, FUN_00401441, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
