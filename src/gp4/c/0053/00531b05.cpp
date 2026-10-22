#include <gp4/gp4.h>

// FUNCTION: GP4 0x00531b05
// A bare `ret`: every register and flag comes back as the caller passed it (the caller at
// 0x542815 goes on to read eax).
void __cdecl FUN_00531b05(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x00531b05, FUN_00531b05, "regs(eax:u32) -> (eax)")
