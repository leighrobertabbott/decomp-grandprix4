#include <gp4/gp4.h>

// Calls 0x005ae990, then forwards (b, c, 1, a) to the library routine 0x005dc726 and returns its result.
// FUNCTION: GP4 0x004d4790
uint32_t __cdecl FUN_004d4790(uint32_t a, uint32_t b, uint32_t c) {
    gp4::cdecl_<0x005ae990, void>();
    return gp4::cdecl_<0x005dc726, uint32_t>(b, c, 1, a);
}
GP4_IMPL(0x004d4790, FUN_004d4790, "cdecl(u32, u32, u32) -> u32")
