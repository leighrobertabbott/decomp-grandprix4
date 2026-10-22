#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041a764
// `mov byte [0x623ada], 0; ret`: clears the byte flag; every register and flag comes back as the
// caller passed it (the caller at 0x5133a7 goes on to read eax).
void __cdecl FUN_0041a764(gp4::Regs* r) {
    (void)r;
    GP4_GLOBAL(uint8_t, 0x00623ada) = 0u;
}
GP4_IMPL(0x0041a764, FUN_0041a764, "regs(eax:u32) -> (eax); globals=fuzz:i32")
