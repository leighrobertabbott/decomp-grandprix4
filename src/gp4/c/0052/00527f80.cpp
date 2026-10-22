#include <gp4/gp4.h>

// FUNCTION: GP4 0x00527f80
// A bare `ret`: every register and flag comes back as the caller passed it (the caller at
// 0x5330b5 goes on to read eax).
void __cdecl FUN_00527f80(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x00527f80, FUN_00527f80, "regs(eax:u32) -> (eax)")
