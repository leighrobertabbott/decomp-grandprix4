#include <gp4/gp4.h>

// FUNCTION: GP4 0x00405f76
// A bare `ret`: every register and flag comes back as the caller passed it (the caller at
// 0x405e79 goes on to read eax).
void __cdecl FUN_00405f76(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x00405f76, FUN_00405f76, "regs(eax:u32) -> (eax)")
