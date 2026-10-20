#include <gp4/gp4.h>

// Stores al into the byte global 0x0088fd9f (`mov byte [0x88fd9f], al; ret`); flags untouched.
// FUNCTION: GP4 0x0051fbbf
void __cdecl FUN_0051fbbf(gp4::Regs* r) {
    const uint8_t value = static_cast<uint8_t>(r->eax);
    GP4_GLOBAL(uint8_t, 0x0088fd9f) = value;
}
GP4_IMPL(0x0051fbbf, FUN_0051fbbf, "regs(eax:u32) -> ()")
