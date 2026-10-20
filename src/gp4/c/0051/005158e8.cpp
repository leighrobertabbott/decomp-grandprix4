#include <gp4/gp4.h>

// Byte setter: stores the four consecutive configuration bytes 6, 2, 0, 0 at 0x0088fd94..0x0088fd97.
// FUNCTION: GP4 0x005158e8
void __cdecl FUN_005158e8() {
    GP4_GLOBAL(uint8_t, 0x0088fd94) = 6;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 2;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 0;
}
GP4_IMPL(0x005158e8, FUN_005158e8, "cdecl() -> void; globals=fuzz:i32")
