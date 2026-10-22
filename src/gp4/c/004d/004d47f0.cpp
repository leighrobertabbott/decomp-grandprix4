#include <gp4/gp4.h>

// Calls the empty routine 0x005ae990, then returns the pointer global at
// 0x006604a8; the code falls through into the one-byte `ret` at 0x004d47fa.
// FUNCTION: GP4 0x004d47f0
uint32_t __cdecl FUN_004d47f0() {
    gp4::cdecl_<0x005ae990, void>();
    uint32_t result = GP4_GLOBAL(uint32_t, 0x006604a8);
    gp4::cdecl_<0x004d47fa, void>();
    return result;
}
GP4_IMPL(0x004d47f0, FUN_004d47f0, "cdecl() -> u32; globals=fuzz:i32")
