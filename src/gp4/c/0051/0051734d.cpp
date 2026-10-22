#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051734d
// A bare `ret`: every register and flag comes back as the caller passed it (the caller at
// 0x517012 goes on to read eax).
void __cdecl FUN_0051734d(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x0051734d, FUN_0051734d, "regs(eax:u32) -> (eax)")
