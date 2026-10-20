#include <gp4/gp4.h>

// Stores the AND of the words 0x007ad892 and 0x00679b5a to the word 0x0066d8b4. eax is saved and
// restored around the 16-bit work.
// FUNCTION: GP4 0x00522435
void __cdecl FUN_00522435() {
    const uint16_t a = GP4_GLOBAL(uint16_t, 0x007ad892);
    const uint16_t b = GP4_GLOBAL(uint16_t, 0x00679b5a);
    GP4_GLOBAL(uint16_t, 0x0066d8b4) = static_cast<uint16_t>(a & b);
}
GP4_IMPL(0x00522435, FUN_00522435, "cdecl() -> void; globals=fuzz:i32")
