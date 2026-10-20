#include <gp4/gp4.h>

// FUNCTION: GP4 0x0054d6f4
void __cdecl FUN_0054d6f4(gp4::Regs*) {
    // push edx / push esi / pop esi / pop edx / ret: every register and flag is preserved.
}
GP4_IMPL(0x0054d6f4, FUN_0054d6f4, "regs() -> ()")
