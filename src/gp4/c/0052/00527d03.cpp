#include <gp4/gp4.h>

// Sets the carry flag (`stc; ret`); every other flag is untouched.
// FUNCTION: GP4 0x00527d03
void __cdecl FUN_00527d03(gp4::Regs* r) {
    r->eflags |= 1u;
}
GP4_IMPL(0x00527d03, FUN_00527d03, "regs() -> (cf)")
