#include <gp4/gp4.h>

// FUNCTION: GP4 0x004014ba
void __cdecl FUN_004014ba(gp4::Regs* r) {
    const uint32_t incomingEax = r->eax;
    const uint16_t lowWord = GP4_GLOBAL(uint16_t, 0x0062d1b4);
    const uint16_t highWord = GP4_GLOBAL(uint16_t, 0x0062d1b6);
    const uint32_t state = GP4_GLOBAL(uint32_t, 0x0062d1b4);
    const uint8_t previous = GP4_GLOBAL(uint8_t, 0x0062d1b3);
    const uint16_t firstShift = lowWord >> 4;
    const uint8_t feedback = static_cast<uint8_t>(firstShift ^ (highWord >> 1));
    const uint32_t shifted = state << 8;
    const uint32_t carry = (state >> 24) & 1u;
    const uint32_t sign = (shifted >> 31) & 1u;
    // OF for a shift by eight is undefined architecturally; this models
    // the pinned Unicorn oracle's penultimate/final shift comparison.
    const uint32_t flags = carry | 4u
        | (shifted == 0u ? 0x40u : 0u)
        | (sign << 7)
        | ((sign ^ carry) << 11);
    GP4_GLOBAL(uint32_t, 0x0062d1b4) = shifted | previous;
    GP4_GLOBAL(uint8_t, 0x0062d1b3) = feedback;
    r->eax = (incomingEax & 0xffff0000u) | (firstShift & 0xff00u) | feedback;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004014ba, FUN_004014ba, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0062d1b4")
