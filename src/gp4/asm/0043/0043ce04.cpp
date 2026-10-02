#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043ce04
void __cdecl FUN_0043ce04(gp4::Regs* r) {
    const uint32_t incomingEax = r->eax;
    const uint32_t incomingEdx = r->edx;
    const uint32_t low = incomingEdx & 0xffu;
    // or dl, dl: CF=OF=AF=0
    const uint32_t orFlags = parityFlag(low) | (low == 0u ? 0x40u : 0u) | (low & 0x80u);
    r->eflags = (r->eflags & ~0x8d5u) | orFlags;
    if ((low & 0x80u) != 0u) {
        // js: the routine returns with the flags of the or
        return;
    }
    // push edx / push eax around the call; 0x401462 only changes eax and the flags
    gp4::call_regs(0x00401462, *r);
    // and eax, 0xff ; shr eax, 1 ; neg eax ; pop edx (= incoming eax) ; add eax, edx
    const uint32_t negated = 0u - ((r->eax & 0xffu) >> 1);
    const uint32_t result = negated + incomingEax;
    const uint32_t flags = (result < negated ? 1u : 0u)
        | parityFlag(result)
        | (((negated ^ incomingEax ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(negated ^ incomingEax) & (negated ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eax = result;
    r->edx = incomingEdx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ce04, FUN_0043ce04, "regs(eax:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
