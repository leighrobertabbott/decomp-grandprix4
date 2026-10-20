#include <gp4/gp4.h>

// Clears the fields at 0x00826d15 (byte), 0x00826d16 and 0x00826d25 (dwords) and the 0x1b bytes from
// 0x00826d2a (`rep stosb` of al = 0). Everything is inside pushal/popal and no flag is written, so
// registers and flags are unchanged.
// FUNCTION: GP4 0x0052e976
void __cdecl FUN_0052e976(gp4::Regs* r) {
    (void)r;
    GP4_GLOBAL(uint8_t, 0x00826d15) = 0;
    GP4_GLOBAL(uint32_t, 0x00826d16) = 0;
    GP4_GLOBAL(uint32_t, 0x00826d25) = 0;
    uint8_t* tail = GP4_ARRAY(uint8_t, 0x00826d2a);
    for (uint32_t i = 0; i != 0x1bu; ++i) {
        tail[i] = 0;
    }
}
GP4_IMPL(0x0052e976, FUN_0052e976, "regs() -> (); globals=fuzz:i32")
