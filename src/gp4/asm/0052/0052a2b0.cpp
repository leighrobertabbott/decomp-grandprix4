#include <gp4/gp4.h>

// pushal; copies the 4 bytes at 0x0081d1d8 back to 0x0081d1d4 (`rep movsb`, direction flag clear; the
// inverse of 0x0052a29c); popal restores every register. The flags are untouched.
// FUNCTION: GP4 0x0052a2b0
void __cdecl FUN_0052a2b0(gp4::Regs* r) {
    (void)r;
    const uint8_t* src = GP4_ARRAY(uint8_t, 0x0081d1d8);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x0081d1d4);
    for (uint32_t i = 0; i != 4u; ++i) {
        dst[i] = src[i];
    }
}
GP4_IMPL(0x0052a2b0, FUN_0052a2b0, "regs() -> (); globals=fuzz:i32")
