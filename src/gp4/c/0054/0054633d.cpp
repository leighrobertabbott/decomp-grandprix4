#include <gp4/gp4.h>

// FUNCTION: GP4 0x0054633d
void __cdecl FUN_0054633d(gp4::Regs*) {
    // push eax / push ebx / pop ebx / pop eax / ret: every register and flag is preserved.
}
GP4_IMPL(0x0054633d, FUN_0054633d, "regs() -> ()")
