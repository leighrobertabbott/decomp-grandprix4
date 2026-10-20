#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051e405
// Force-feedback gate: when none of the three status bytes 0x852d6a, 0x88f63a and 0x852d98 is set,
// the latch byte 0x6f2c29 is cleared; otherwise, unless the latch is already 0xff (return at once),
// the two force pairs 0x6f2d54/0x6f2d58 and 0x6f2d64/0x6f2d68 are zeroed and the latch is set to
// 0xff. Both non-returning paths then run the effect update 0x51e380.
void __cdecl FUN_0051e405(void) {
    if (GP4_GLOBAL(uint8_t, 0x00852d6a) == 0u && GP4_GLOBAL(uint8_t, 0x0088f63a) == 0u &&
        GP4_GLOBAL(uint8_t, 0x00852d98) == 0u) {
        GP4_GLOBAL(uint8_t, 0x006f2c29) = 0u;
    } else {
        if (GP4_GLOBAL(uint8_t, 0x006f2c29) == 0xffu) {
            return;
        }
        GP4_GLOBAL(uint32_t, 0x006f2d54) = 0u;
        GP4_GLOBAL(uint32_t, 0x006f2d58) = 0u;
        GP4_GLOBAL(uint32_t, 0x006f2d64) = 0u;
        GP4_GLOBAL(uint32_t, 0x006f2d68) = 0u;
        GP4_GLOBAL(uint8_t, 0x006f2c29) = 0xffu;
    }
    gp4::cdecl_<0x0051e380, void>();
}
GP4_IMPL(0x0051e405, FUN_0051e405, "cdecl() -> void; globals=layout=cf_0041a2b2+ffb_0051e380")
