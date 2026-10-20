#include <gp4/gp4.h>

// Stores eax into the dword global at 0x0082fd70 (`mov [0x82fd70], eax; ret`).
// FUNCTION: GP4 0x0053e626
void __cdecl FUN_0053e626(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0082fd70) = r->eax;
}
GP4_IMPL(0x0053e626, FUN_0053e626, "regs(eax:u32) -> ()")
