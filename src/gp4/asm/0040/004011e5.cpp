#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004011e5
// ax = 16-bit angle: table interpolation at 0x007c5c5c. Returns the 32-bit
// fraction (delta * (angle & 7)) << 13 in dx:ax, with the base word added to dx.
void __cdecl FUN_004011e5(gp4::Regs* r) {
    uint32_t angle = r->eax & 0xffffu;
    if ((angle & 0x8000u) != 0u) angle = (0u - angle) & 0xffffu;

    const uint32_t offset = (angle >> 2) & 0xfffeu;
    const int16_t next = *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5e) + offset);
    const int16_t base = *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + offset);

    const int16_t delta = static_cast<int16_t>(next - base);
    const int32_t product = static_cast<int32_t>(delta) * static_cast<int32_t>(angle & 7u);
    // byte shuffle (<< 8) followed by five shl/rcl pairs (<< 5): 32-bit product << 13
    const uint32_t shifted = static_cast<uint32_t>(product) << 13;

    const uint32_t left = shifted >> 16;
    const uint32_t right = static_cast<uint16_t>(base);
    const uint32_t sum = left + right;
    const uint32_t result = sum & 0xffffu;
    const uint32_t flags = (sum > 0xffffu ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);

    r->eax = (r->eax & 0xffff0000u) | (shifted & 0xffffu);
    r->edx = (r->edx & 0xffff0000u) | result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004011e5, FUN_004011e5, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of)")
