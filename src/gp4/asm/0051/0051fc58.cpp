#include <gp4/gp4.h>

// Clears byte [eax] of the byte table at 0x00866e47 (eax = index in a byte
// register, see the caller 0x004440be).
// FUNCTION: GP4 0x0051fc58
void __cdecl FUN_0051fc58(gp4::Regs* r) {
    const uint32_t index = r->eax;
    GP4_ARRAY(uint8_t, 0x00866e47)[index] = 0u;
}
GP4_IMPL(0x0051fc58, FUN_0051fc58, "regs(eax:u32[0..63]) -> ()")
