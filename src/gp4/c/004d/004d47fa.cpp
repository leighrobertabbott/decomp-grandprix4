#include <gp4/gp4.h>

// A bare `ret`: touches nothing. Callers read eax after the call, so the
// register block is declared as passing through unchanged.
// FUNCTION: GP4 0x004d47fa
void __cdecl FUN_004d47fa(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x004d47fa, FUN_004d47fa, "regs(eax:u32) -> (eax)")
