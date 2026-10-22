#include <gp4/gp4.h>

// FUNCTION: GP4 0x00531b37
// A bare `ret`: every register and flag comes back as the caller passed it (the caller at
// 0x53aa75 goes on to read eax).
void __cdecl FUN_00531b37(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x00531b37, FUN_00531b37, "regs(eax:u32) -> (eax)")
