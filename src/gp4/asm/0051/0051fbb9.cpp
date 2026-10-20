#include <gp4/gp4.h>

// Stores al into the byte global 0x0088fda0 (`mov byte [0x88fda0], al; ret`); flags untouched.
// FUNCTION: GP4 0x0051fbb9
void __cdecl FUN_0051fbb9(gp4::Regs* r) {
    const uint8_t value = static_cast<uint8_t>(r->eax);
    GP4_GLOBAL(uint8_t, 0x0088fda0) = value;
}
GP4_IMPL(0x0051fbb9, FUN_0051fbb9, "regs(eax:u32) -> ()")
