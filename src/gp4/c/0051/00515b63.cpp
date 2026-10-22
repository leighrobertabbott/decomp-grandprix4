#include <gp4/gp4.h>

// FUNCTION: GP4 0x00515b63
// A bare `ret`: every register and flag comes back as the caller passed it (the caller at
// 0x51582d goes on to read eax).
void __cdecl FUN_00515b63(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x00515b63, FUN_00515b63, "regs(eax:u32) -> (eax)")
