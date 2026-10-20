#include <gp4/gp4.h>

// Stores al into the byte global at 0x0088fda0 (`mov [0x88fda0], al; ret`).
// FUNCTION: GP4 0x0051d48e
void __cdecl FUN_0051d48e(gp4::Regs* r) {
    const uint8_t value = static_cast<uint8_t>(r->eax);
    GP4_GLOBAL(uint8_t, 0x0088fda0) = value;
}
GP4_IMPL(0x0051d48e, FUN_0051d48e, "regs(eax:u32) -> ()")
