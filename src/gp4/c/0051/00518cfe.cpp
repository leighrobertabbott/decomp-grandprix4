#include <gp4/gp4.h>

// FUNCTION: GP4 0x00518cfe
void __cdecl FUN_00518cfe(gp4::Regs*) {
    // push eax / pop eax / ret: every register and flag is preserved.
}
GP4_IMPL(0x00518cfe, FUN_00518cfe, "regs() -> ()")
