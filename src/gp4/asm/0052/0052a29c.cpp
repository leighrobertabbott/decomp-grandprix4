#include <gp4/gp4.h>

// pushal; copies the 4 bytes at 0x0081d1d4 to 0x0081d1d8 (`rep movsb`, direction flag clear); popal
// restores every register. The flags are untouched.
// FUNCTION: GP4 0x0052a29c
void __cdecl FUN_0052a29c(gp4::Regs* r) {
    (void)r;
    const uint8_t* src = GP4_ARRAY(uint8_t, 0x0081d1d4);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x0081d1d8);
    for (uint32_t i = 0; i != 4u; ++i) {
        dst[i] = src[i];
    }
}
GP4_IMPL(0x0052a29c, FUN_0052a29c, "regs() -> (); globals=fuzz:i32")
