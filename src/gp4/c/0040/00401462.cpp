#include <gp4/gp4.h>

// FUNCTION: GP4 0x00401462
void __cdecl FUN_00401462(gp4::Regs* r) {
    const uint32_t incomingEax = r->eax;
    const uint16_t lowWord = GP4_GLOBAL(uint16_t, 0x0066d8c0);
    const uint16_t highWord = GP4_GLOBAL(uint16_t, 0x0066d8c2);
    const uint32_t state = GP4_GLOBAL(uint32_t, 0x0066d8c0);
    const uint8_t previous = GP4_GLOBAL(uint8_t, 0x0066d8bf);
    const uint16_t firstShift = lowWord >> 4;
    const uint8_t feedback = static_cast<uint8_t>(firstShift ^ (highWord >> 1));
    const uint32_t shifted = state << 8;
    const uint32_t carry = (state >> 24) & 1u;
    const uint32_t sign = (shifted >> 31) & 1u;
    // OF after a shift by eight is architecturally undefined. This matches
    // the pinned Unicorn model's penultimate/final shifted-value comparison.
    const uint32_t flags = carry | 4u
        | (shifted == 0u ? 0x40u : 0u)
        | (sign << 7)
        | ((sign ^ carry) << 11);
    GP4_GLOBAL(uint32_t, 0x0066d8c0) = shifted | previous;
    GP4_GLOBAL(uint8_t, 0x0066d8bf) = feedback;
    r->eax = (incomingEax & 0xffff0000u) | (firstShift & 0xff00u) | feedback;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00401462, FUN_00401462, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
