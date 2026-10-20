#include <gp4/gp4.h>

// Tests bit [0x007ab022] of the bit string at 0x0062f8c8 (BT with a register bit offset addresses whole dwords
// beyond the first) and stores 0 to the byte 0x0062f8ee when the bit is set, 0xff when it is clear. eax ends as
// the zero-extended bit index; only CF (the tested bit) changes in the flags.
// FUNCTION: GP4 0x00445a2c
void __cdecl FUN_00445a2c(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint8_t, 0x007ab022);
    const uint32_t word = GP4_ARRAY(uint32_t, 0x0062f8c8)[index >> 5];
    const uint32_t bit = (word >> (index & 31u)) & 1u;

    GP4_GLOBAL(uint8_t, 0x0062f8ee) = bit != 0u ? 0x00u : 0xffu;
    r->eax = index;
    r->eflags = (r->eflags & ~1u) | bit;
}
GP4_IMPL(0x00445a2c, FUN_00445a2c, "regs() -> (eax, cf); globals=layout=bits_0062f8c8")
