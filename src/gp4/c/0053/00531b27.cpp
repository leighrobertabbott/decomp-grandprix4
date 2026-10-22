#include <gp4/gp4.h>

// FUNCTION: GP4 0x00531b27
// A bare `ret`: every register and flag comes back as the caller passed it (the caller at
// 0x52a7db goes on to read eax).
void __cdecl FUN_00531b27(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x00531b27, FUN_00531b27, "regs(eax:u32) -> (eax)")
