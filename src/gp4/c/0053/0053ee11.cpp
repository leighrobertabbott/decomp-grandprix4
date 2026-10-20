#include <gp4/gp4.h>

// Loads al from the byte global 0x00822272 (`mov al, [0x822272]; ret`); the upper 24 bits of eax
// and the flags are untouched.
// FUNCTION: GP4 0x0053ee11
void __cdecl FUN_0053ee11(gp4::Regs* r) {
    r->eax = (r->eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00822272);
}
GP4_IMPL(0x0053ee11, FUN_0053ee11, "regs(eax:u32) -> (eax); globals=fuzz:i32")
