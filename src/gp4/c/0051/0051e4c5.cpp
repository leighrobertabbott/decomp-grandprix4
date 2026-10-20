#include <gp4/gp4.h>

// Zeroes the force values of both effect slots (0x006f2d54/58 and 0x006f2d64/68), then runs the
// force-feedback update 0x0051e380 so the effects are stopped.
// FUNCTION: GP4 0x0051e4c5
void __cdecl FUN_0051e4c5() {
    GP4_GLOBAL(uint32_t, 0x006f2d54) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f2d58) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f2d64) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f2d68) = 0u;
    gp4::cdecl_<0x0051e380, void>();
}
GP4_IMPL(0x0051e4c5, FUN_0051e4c5, "cdecl() -> void; globals=layout=cf_0041a2b2+ffb_0051e380")
