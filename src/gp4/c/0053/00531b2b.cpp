#include <gp4/gp4.h>

// FUNCTION: GP4 0x00531b2b
// A bare `ret`: every register and flag comes back as the caller passed it (the caller at
// 0x53b850 goes on to read eax).
void __cdecl FUN_00531b2b(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x00531b2b, FUN_00531b2b, "regs(eax:u32) -> (eax)")
