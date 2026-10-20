#include <gp4/gp4.h>

// pushal / mov [0xa6987c], eax / popal / ret: stores eax into a global.
// FUNCTION: GP4 0x0051f8d6
void __cdecl FUN_0051f8d6(gp4::Regs* r) {
    const uint32_t value = r->eax;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = value;
}
GP4_IMPL(0x0051f8d6, FUN_0051f8d6, "regs(eax:u32) -> ()")
