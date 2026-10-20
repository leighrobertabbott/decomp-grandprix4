#include <gp4/gp4.h>

// Clears the 32-byte (256-bit) bitmap at 0x008245a0 (the "wanted" bit set read by 0x00510e2d). eax, ecx
// and edi are saved and restored around the `rep stosb`; the flags are untouched.
// FUNCTION: GP4 0x0052d6d8
void __cdecl FUN_0052d6d8(gp4::Regs* /*r*/) {
    uint32_t* bitmap = GP4_ARRAY(uint32_t, 0x008245a0);
    for (uint32_t i = 0u; i < 8u; ++i) {
        bitmap[i] = 0u;
    }
}
GP4_IMPL(0x0052d6d8, FUN_0052d6d8, "regs() -> (); globals=fuzz:i32")
