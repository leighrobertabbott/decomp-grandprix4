#include <gp4/gp4.h>

// Adds the word [0x00852de8] to the word [0x00679b58] (wrapping at 16 bits) and calls 0x0051f293 with eax = 2 when bit 7
// of the new word is set, else 0 (edx carries the new word). Called from C code (0x004425e0, 0x00442988) whose next
// instructions do not read the flags, like 0x00442cea: modelled as cdecl.
// FUNCTION: GP4 0x00442cf5
uint32_t __cdecl FUN_00442cf5() {
    const uint16_t sum = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00679b58) + GP4_GLOBAL(uint16_t, 0x00852de8));
    GP4_GLOBAL(uint16_t, 0x00679b58) = sum;
    gp4::Regs c = {};
    c.edx = sum;
    c.eax = (sum & 0x80u) != 0u ? 2u : 0u;
    gp4::call_regs(0x0051f293, c);
    return c.eax;
}
GP4_IMPL(0x00442cf5, FUN_00442cf5, "cdecl() -> u32; globals=fuzz:i32")
