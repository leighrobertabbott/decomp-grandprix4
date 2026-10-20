#include <gp4/gp4.h>

// Resets the dword 0x00823690 to 0 and sets the dword 0x007ad898 to 4.
// FUNCTION: GP4 0x0053f7ca
void __cdecl FUN_0053f7ca() {
    GP4_GLOBAL(uint32_t, 0x00823690) = 0u;
    GP4_GLOBAL(uint32_t, 0x007ad898) = 4u;
}
GP4_IMPL(0x0053f7ca, FUN_0053f7ca, "cdecl() -> void; globals=fuzz:i32")
