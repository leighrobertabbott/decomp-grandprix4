#include <gp4/gp4.h>

// Stores al into the byte global at 0x0088fd9f (`mov [0x88fd9f], al; ret`).
// FUNCTION: GP4 0x0051d494
void __cdecl FUN_0051d494(gp4::Regs* r) {
    const uint8_t value = static_cast<uint8_t>(r->eax);
    GP4_GLOBAL(uint8_t, 0x0088fd9f) = value;
}
GP4_IMPL(0x0051d494, FUN_0051d494, "regs(eax:u32) -> ()")
