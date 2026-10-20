#include <gp4/gp4.h>

// Clears the carry flag (`clc; ret`); every other flag is untouched.
// FUNCTION: GP4 0x00531741
void __cdecl FUN_00531741(gp4::Regs* r) {
    r->eflags &= ~1u;
}
GP4_IMPL(0x00531741, FUN_00531741, "regs() -> (cf)")
