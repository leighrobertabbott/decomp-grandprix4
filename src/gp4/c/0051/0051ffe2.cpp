#include <gp4/gp4.h>

// Resets four 16-bit words at 0x0066d882..0x0066d888: [0x66d888] = 0, the other three = 0x8000.
// FUNCTION: GP4 0x0051ffe2
void __cdecl FUN_0051ffe2() {
    GP4_GLOBAL(uint16_t, 0x0066d888) = 0u;
    GP4_GLOBAL(uint16_t, 0x0066d884) = 0x8000u;
    GP4_GLOBAL(uint16_t, 0x0066d882) = 0x8000u;
    GP4_GLOBAL(uint16_t, 0x0066d886) = 0x8000u;
}
GP4_IMPL(0x0051ffe2, FUN_0051ffe2, "cdecl() -> void; globals=fuzz:i32")
