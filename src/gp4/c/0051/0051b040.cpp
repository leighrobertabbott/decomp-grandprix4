#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051b040
// A bare `ret`: every register and flag comes back as the caller passed it (the caller at
// 0x51bf1f goes on to read eax).
void __cdecl FUN_0051b040(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x0051b040, FUN_0051b040, "regs(eax:u32) -> (eax)")
