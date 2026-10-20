#include <gp4/gp4.h>

// Clears the carry flag (`clc; ret`); every other flag is untouched.
// FUNCTION: GP4 0x0054a4a5
void __cdecl FUN_0054a4a5(gp4::Regs* r) {
    r->eflags &= ~1u;
}
GP4_IMPL(0x0054a4a5, FUN_0054a4a5, "regs() -> (cf)")
