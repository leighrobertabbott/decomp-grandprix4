#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// 16-bit linear congruential step: the word at 0x0066d8c4 is multiplied by 5 (mul dx, with dx = 5, so dx receives
// the high word of the product) and the new value's bits 2..7 are returned in ax. The final "and ax, 0x3f" sets the
// flags (CF = OF = AF = 0); the upper halves of eax and edx are preserved.
// FUNCTION: GP4 0x004431b3
void __cdecl FUN_004431b3(gp4::Regs* r) {
    const uint32_t product = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x0066d8c4)) * 5u;
    const uint16_t low = static_cast<uint16_t>(product);
    const uint16_t high = static_cast<uint16_t>(product >> 16);
    GP4_GLOBAL(uint16_t, 0x0066d8c4) = low;

    const uint32_t result = static_cast<uint32_t>(low >> 2) & 0x3fu;
    r->eax = (r->eax & 0xffff0000u) | result;
    r->edx = (r->edx & 0xffff0000u) | high;
    r->eflags = (r->eflags & ~0x8d5u) | byteParity(result) | (result == 0u ? 0x40u : 0u);
}
GP4_IMPL(0x004431b3, FUN_004431b3, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
