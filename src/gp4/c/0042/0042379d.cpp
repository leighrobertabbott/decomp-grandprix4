#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042379d
// Clears the six 64-bit accumulators at 0x00607ad0..0x00607afc (counterpart of
// the accumulate routine at 0x00423816).
void __cdecl FUN_0042379d() {
    GP4_GLOBAL(uint32_t, 0x00607ad0) = 0;
    GP4_GLOBAL(uint32_t, 0x00607ad4) = 0;
    GP4_GLOBAL(uint32_t, 0x00607ad8) = 0;
    GP4_GLOBAL(uint32_t, 0x00607adc) = 0;
    GP4_GLOBAL(uint32_t, 0x00607ae0) = 0;
    GP4_GLOBAL(uint32_t, 0x00607ae4) = 0;
    GP4_GLOBAL(uint32_t, 0x00607ae8) = 0;
    GP4_GLOBAL(uint32_t, 0x00607aec) = 0;
    GP4_GLOBAL(uint32_t, 0x00607af0) = 0;
    GP4_GLOBAL(uint32_t, 0x00607af4) = 0;
    GP4_GLOBAL(uint32_t, 0x00607af8) = 0;
    GP4_GLOBAL(uint32_t, 0x00607afc) = 0;
}
GP4_IMPL(0x0042379d, FUN_0042379d, "cdecl() -> void; globals=fuzz:i32")
