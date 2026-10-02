#include <gp4/gp4.h>

// FUNCTION: GP4 0x00421ef8
void __cdecl FUN_00421ef8() {
    GP4_GLOBAL(uint32_t, 0x006080fc) = 0;
    GP4_GLOBAL(uint32_t, 0x00608100) = 0;
    GP4_GLOBAL(uint32_t, 0x00608104) = 0;
    GP4_GLOBAL(uint32_t, 0x00608114) = 0;
    GP4_GLOBAL(uint32_t, 0x00608118) = 0;
    GP4_GLOBAL(uint32_t, 0x0060811c) = 0;
    GP4_GLOBAL(uint32_t, 0x00608120) = 0;
    GP4_GLOBAL(uint32_t, 0x00608124) = 0;
    GP4_GLOBAL(uint32_t, 0x00608128) = 0;
}
GP4_IMPL(0x00421ef8, FUN_00421ef8, "cdecl() -> void; globals=fuzz:i32")
