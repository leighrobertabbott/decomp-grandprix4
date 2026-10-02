#include <gp4/gp4.h>

// FUNCTION: GP4 0x00410934
void __cdecl FUN_00410934(gp4::Regs* r) {
    const uint32_t input = r->eax & 0xffu;
    const uint32_t index = r->edx < 17u ? r->edx : 0u;
    const uint32_t entry = GP4_ARRAY(uint8_t, 0x0062eade)[index];
    const uint32_t divisor = entry == 0u ? 1u : entry;
    const uint32_t quotient = (input * 200u) / divisor;
    const uint32_t half = quotient >> 1;
    const uint32_t carry = quotient & 1u;
    const uint32_t rounded = half + carry;
    uint32_t parity = rounded & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = ((parity & 1u) == 0u ? 4u : 0u)
        | (((half ^ carry ^ rounded) & 0x10u) != 0u ? 0x10u : 0u)
        | (rounded == 0u ? 0x40u : 0u);
    r->eax = rounded;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00410934, FUN_00410934, "regs(eax:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=scale_0062eade")
