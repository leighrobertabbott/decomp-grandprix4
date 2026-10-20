#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040155b
// Copies the three words at 0x00629bca/cc/ce to 0x0066d8be/c0/c2.
void __cdecl FUN_0040155b() {
    GP4_GLOBAL(uint16_t, 0x0066d8be) = GP4_GLOBAL(uint16_t, 0x00629bca);
    GP4_GLOBAL(uint16_t, 0x0066d8c0) = GP4_GLOBAL(uint16_t, 0x00629bcc);
    GP4_GLOBAL(uint16_t, 0x0066d8c2) = GP4_GLOBAL(uint16_t, 0x00629bce);
}
GP4_IMPL(0x0040155b, FUN_0040155b, "cdecl() -> void; globals=fuzz:i32")
