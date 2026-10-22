#include <gp4/gp4.h>

// Copies six byte-sized state globals (0x00a66388, 0x00a663cc, 0x00a663ac, 0x00a66368,
// 0x00a663bc, 0x00a664b4) into the consecutive bytes 0x007ad835..0x007ad83a and clears the dword
// global 0x00abc0f0.
// FUNCTION: GP4 0x005012a0
void __cdecl FUN_005012a0() {
    uint8_t a = GP4_GLOBAL(uint8_t, 0x00a66388);
    uint8_t b = GP4_GLOBAL(uint8_t, 0x00a663cc);
    uint8_t c = GP4_GLOBAL(uint8_t, 0x00a663ac);
    GP4_GLOBAL(uint8_t, 0x007ad835) = a;
    a = GP4_GLOBAL(uint8_t, 0x00a66368);
    GP4_GLOBAL(uint8_t, 0x007ad836) = b;
    b = GP4_GLOBAL(uint8_t, 0x00a663bc);
    GP4_GLOBAL(uint8_t, 0x007ad837) = c;
    c = GP4_GLOBAL(uint8_t, 0x00a664b4);
    GP4_GLOBAL(uint8_t, 0x007ad838) = a;
    GP4_GLOBAL(uint8_t, 0x007ad839) = b;
    GP4_GLOBAL(uint8_t, 0x007ad83a) = c;
    GP4_GLOBAL(uint32_t, 0x00abc0f0) = 0;
}
GP4_IMPL(0x005012a0, FUN_005012a0, "cdecl() -> void; globals=fuzz:i32")
