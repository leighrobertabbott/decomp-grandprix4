#include <gp4/gp4.h>

// Copies the byte at 0x007aafd3 to 0x007aafd4 and clears the byte at 0x007aafd8. eax is pushed and
// popped around it (al is the scratch register) and the flags are untouched.
// FUNCTION: GP4 0x0053257c
void __cdecl FUN_0053257c(gp4::Regs* r) {
    (void)r;
    GP4_GLOBAL(uint8_t, 0x007aafd4) = GP4_GLOBAL(uint8_t, 0x007aafd3);
    GP4_GLOBAL(uint8_t, 0x007aafd8) = 0u;
}
GP4_IMPL(0x0053257c, FUN_0053257c, "regs() -> (); globals=fuzz:i32")
