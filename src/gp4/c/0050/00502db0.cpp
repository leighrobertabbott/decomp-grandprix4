#include <gp4/gp4.h>

// Initialises the block of globals at 0x0066b0e4..0x0066b10c: the pointer stored at 0x0066a668 is
// copied to 0x0066b0e4, 0x0066b0f0 and 0x0066b0fc, the dwords at 0x0066b0e8/f4/100 get 0x0d,
// 0x0066b0ec gets 1, 0x0066b0f8 gets 0x19c, and 0x0066b104/108/10c are cleared.
// FUNCTION: GP4 0x00502db0
void __cdecl FUN_00502db0() {
    uint32_t source = GP4_GLOBAL(uint32_t, 0x0066a668);
    GP4_GLOBAL(uint32_t, 0x0066b0e4) = source;
    GP4_GLOBAL(uint32_t, 0x0066b0f0) = source;
    GP4_GLOBAL(uint32_t, 0x0066b0fc) = source;
    GP4_GLOBAL(uint32_t, 0x0066b0e8) = 0x0d;
    GP4_GLOBAL(uint32_t, 0x0066b0ec) = 1;
    GP4_GLOBAL(uint32_t, 0x0066b0f4) = 0x0d;
    GP4_GLOBAL(uint32_t, 0x0066b0f8) = 0x19c;
    GP4_GLOBAL(uint32_t, 0x0066b100) = 0x0d;
    GP4_GLOBAL(uint32_t, 0x0066b104) = 0;
    GP4_GLOBAL(uint32_t, 0x0066b108) = 0;
    GP4_GLOBAL(uint32_t, 0x0066b10c) = 0;
}
GP4_IMPL(0x00502db0, FUN_00502db0, "cdecl() -> void; globals=fuzz:i32")
