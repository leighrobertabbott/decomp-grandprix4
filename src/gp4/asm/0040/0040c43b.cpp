#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040c43b
void __cdecl FUN_0040c43b(gp4::Regs* r) {
    const uint32_t x = r->eax;
    const uint32_t y = r->edx;
    const uint32_t hi = r->ebx;

    // Two 32-bit fixed-point coordinates (screen-space x and negated y) offset by
    // the origin dwords; the top byte of each selects a cell of the lookup table.
    const uint32_t xPos = (x << 13) + ((GP4_GLOBAL(uint32_t, 0x0062aca4) << 23) + GP4_GLOBAL(uint32_t, 0x0063b828));
    const uint32_t yPos = ((0u - y) << 13) + ((GP4_GLOBAL(uint32_t, 0x0062aca8) << 23) + GP4_GLOBAL(uint32_t, 0x0063b82c));

    uint32_t index = (hi << 8) | (yPos >> 24);
    index = (index << 8) | (xPos >> 24);
    index &= GP4_GLOBAL(uint32_t, 0x0062aca0);

    int8_t value = *(const int8_t*)(GP4_GLOBAL(uint32_t, 0x0062ac9c) + index);
    if (value >= 6) {
        value = 0;
    }
    const uint32_t result = (uint32_t)(uint8_t)value;
    r->eax = result;

    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    // AND: CF = OF = AF = SF(bit 31 of a byte result) = 0
    r->eflags = (r->eflags & ~0x8d5u) | ((~parity & 1u) << 2) | (result == 0u ? 0x40u : 0u);
}
GP4_IMPL(0x0040c43b, FUN_0040c43b, "regs(eax:u32, ebx:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=map_0040c43b")
