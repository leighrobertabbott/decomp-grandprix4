#include <gp4/gp4.h>

// FUNCTION: GP4 0x004437cc
uint32_t __cdecl FUN_004437cc() {
    return GP4_GLOBAL(uint32_t, 0x0062ad6c) >> 0x12;
}
GP4_IMPL(0x004437cc, FUN_004437cc, "cdecl() -> u32; globals=fuzz:i32")
