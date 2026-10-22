#include <gp4/gp4.h>

// FUNCTION: GP4 0x00531b2e
// A bare `ret`: every register and flag comes back as the caller passed it (the caller at
// 0x52a8c3 goes on to read eax).
void __cdecl FUN_00531b2e(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x00531b2e, FUN_00531b2e, "regs(eax:u32) -> (eax)")
