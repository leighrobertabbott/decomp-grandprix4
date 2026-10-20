#include <gp4/gp4.h>

// FUNCTION: GP4 0x00530774
uint32_t __cdecl FUN_00530774() {
    return GP4_GLOBAL(uint32_t, 0x007ad87c);
}
GP4_IMPL(0x00530774, FUN_00530774, "cdecl() -> u32; globals=fuzz:i32")
