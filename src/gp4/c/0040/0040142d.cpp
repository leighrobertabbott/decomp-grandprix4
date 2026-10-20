#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? 0x4u : 0u;
}

// FUNCTION: GP4 0x0040142d
void __cdecl FUN_0040142d(gp4::Regs* r) {
    gp4::Regs call = *r;

    // First 16-bit value becomes the high half; edx is saved/restored around the pair.
    gp4::call_regs(0x0040140f, call);
    const uint32_t high = call.eax << 16;
    call.eax = high;

    gp4::call_regs(0x0040140f, call);
    const uint32_t result = call.eax | high;

    // or eax, edx: CF = OF = AF = 0, PF/ZF/SF from the 32-bit result.
    const uint32_t flags = parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040142d, FUN_0040142d, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
