#include <gp4/gp4.h>

// Stores al into the byte global at 0x00833cf3 (`mov [0x833cf3], al; ret`).
// FUNCTION: GP4 0x0054cb4a
void __cdecl FUN_0054cb4a(gp4::Regs* r) {
    const uint8_t value = static_cast<uint8_t>(r->eax);
    GP4_GLOBAL(uint8_t, 0x00833cf3) = value;
}
GP4_IMPL(0x0054cb4a, FUN_0054cb4a, "regs(eax:u32) -> ()")
