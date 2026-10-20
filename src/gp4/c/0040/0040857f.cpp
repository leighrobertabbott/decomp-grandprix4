#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040857f
void __cdecl FUN_0040857f() {
    GP4_GLOBAL(uint8_t, 0x0062fb4b) = 0xff;
    if ((GP4_GLOBAL(uint16_t, 0x007ad892) & 0x40) == 0) {
        GP4_GLOBAL(uint8_t, 0x0062fb4b) = 0;
    }
    if (GP4_GLOBAL(uint8_t, 0x007c2eae) != 0) {
        GP4_GLOBAL(uint16_t, 0x007ad892) &= 0xffbf;
        GP4_GLOBAL(uint16_t, 0x007ad892) |= static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x0063b940) & 0x40);
    } else if (GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0x80) {
        GP4_GLOBAL(uint16_t, 0x007ad892) &= 0xffbf;
    }
    GP4_GLOBAL(uint16_t, 0x00603850) = 0;
    GP4_GLOBAL(uint8_t, 0x006f2c28) = 0;
    GP4_GLOBAL(uint8_t, 0x00603758) = 0;
    gp4::cdecl_<0x0040bcb9, void>();
    gp4::cdecl_<0x00433f30, void>();
}
GP4_IMPL(0x0040857f, FUN_0040857f, "cdecl() -> void; globals=fuzz:i32+layout=state_00433faa")
