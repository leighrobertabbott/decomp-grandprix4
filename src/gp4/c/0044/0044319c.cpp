#include <gp4/gp4.h>

// FUNCTION: GP4 0x0044319c
void __cdecl FUN_0044319c() {
    GP4_GLOBAL(uint16_t, 0x0066d8c4) = GP4_GLOBAL(uint16_t, 0x00679b8c);
    GP4_GLOBAL(uint16_t, 0x0066d8c4) |= 1u;
}
GP4_IMPL(0x0044319c, FUN_0044319c, "cdecl() -> void; globals=fuzz:i32")
