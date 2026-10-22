#include <gp4/gp4.h>

// Seeds the C runtime's random generator: passes the result of 0x005dd54f(0) to 0x005db0da
// (both excluded CRT routines; the pair is the usual `srand(time(0))`).
// FUNCTION: GP4 0x005a7a80
void __cdecl FUN_005a7a80() {
    const uint32_t seed = gp4::cdecl_<0x005dd54f, uint32_t>(0);
    gp4::cdecl_<0x005db0da, void>(seed);
}
GP4_IMPL(0x005a7a80, FUN_005a7a80, "cdecl() -> void")
