#include <gp4/gp4.h>

// Clears the top two bits of each of the 22 bytes at 0x007c2a48. ecx and edi are saved and restored.
// FUNCTION: GP4 0x0052d402
void __cdecl FUN_0052d402() {
    uint8_t* p = GP4_ARRAY(uint8_t, 0x007c2a48);
    for (int32_t n = 0x16; n != 0; --n) {
        *p = static_cast<uint8_t>(*p & 0x3fu);
        ++p;
    }
}
GP4_IMPL(0x0052d402, FUN_0052d402, "cdecl() -> void; globals=layout=drv22_0052d402")
