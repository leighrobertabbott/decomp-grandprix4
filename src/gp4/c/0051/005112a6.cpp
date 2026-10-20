#include <gp4/gp4.h>

// Case block of the switch at 0x00511243 (it returns by itself): clears the byte flag at
// 0x007ad8ae and returns with the carry flag set (`stc`); the other flags are untouched.
// FUNCTION: GP4 0x005112a6
void __cdecl FUN_005112a6(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x007ad8ae) = 0u;
    r->eflags |= 1u;
}
GP4_IMPL(0x005112a6, FUN_005112a6, "regs() -> (cf); globals=fuzz:i32")
