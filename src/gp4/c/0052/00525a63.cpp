#include <gp4/gp4.h>

// Clears four state dwords of the 0x006f3aac block: 0x006f3ab8, 0x006f3abc, 0x006f3aac, 0x006f3ab0.
// FUNCTION: GP4 0x00525a63
void __cdecl FUN_00525a63() {
    GP4_GLOBAL(uint32_t, 0x006f3ab8) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f3abc) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f3aac) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f3ab0) = 0u;
}
GP4_IMPL(0x00525a63, FUN_00525a63, "cdecl() -> void; globals=fuzz:i32")
