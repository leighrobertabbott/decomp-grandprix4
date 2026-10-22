#include <gp4/gp4.h>

// Selects the current pointer in 0x006604a8: the argument, or the default block 0x009fb2d4 when
// the argument is null. Returns the argument.
// FUNCTION: GP4 0x004d47d0
uint32_t __cdecl FUN_004d47d0(uint32_t pointer) {
    gp4::cdecl_<0x005ae990, void>();
    GP4_GLOBAL(uint32_t, 0x006604a8) = 0x009fb2d4;
    if (pointer != 0) {
        GP4_GLOBAL(uint32_t, 0x006604a8) = pointer;
    }
    return pointer;
}
GP4_IMPL(0x004d47d0, FUN_004d47d0, "cdecl(u32{0,0x9fb2d4,0x12345678}) -> u32; globals=fuzz:i32")
