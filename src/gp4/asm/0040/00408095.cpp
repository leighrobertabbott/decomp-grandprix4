#include <gp4/gp4.h>

// FUNCTION: GP4 0x00408095
void __cdecl FUN_00408095(gp4::Regs* r) {
    const uint8_t value = static_cast<uint8_t>(r->eax);
    GP4_GLOBAL(uint8_t, 0x006279f8) = value;
}
GP4_IMPL(0x00408095, FUN_00408095, "regs(eax:u32) -> ()")
