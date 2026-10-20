#include <gp4/gp4.h>

// FUNCTION: GP4 0x00401530
// Copies the three words at 0x00629bca/cc/ce to 0x0062d1b2/b4/b6.
void __cdecl FUN_00401530() {
    GP4_GLOBAL(uint16_t, 0x0062d1b2) = GP4_GLOBAL(uint16_t, 0x00629bca);
    GP4_GLOBAL(uint16_t, 0x0062d1b4) = GP4_GLOBAL(uint16_t, 0x00629bcc);
    GP4_GLOBAL(uint16_t, 0x0062d1b6) = GP4_GLOBAL(uint16_t, 0x00629bce);
}
GP4_IMPL(0x00401530, FUN_00401530, "cdecl() -> void; globals=fuzz:i32")
