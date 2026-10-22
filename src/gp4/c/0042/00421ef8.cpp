#include <gp4/gp4.h>

// FUNCTION: GP4 0x00421ef8
// Clears nine dword globals; registers (eax, which callers read) and flags pass through unchanged.
void __cdecl FUN_00421ef8(gp4::Regs* r) {
    (void)r;
    GP4_GLOBAL(uint32_t, 0x006080fc) = 0;
    GP4_GLOBAL(uint32_t, 0x00608100) = 0;
    GP4_GLOBAL(uint32_t, 0x00608104) = 0;
    GP4_GLOBAL(uint32_t, 0x00608114) = 0;
    GP4_GLOBAL(uint32_t, 0x00608118) = 0;
    GP4_GLOBAL(uint32_t, 0x0060811c) = 0;
    GP4_GLOBAL(uint32_t, 0x00608120) = 0;
    GP4_GLOBAL(uint32_t, 0x00608124) = 0;
    GP4_GLOBAL(uint32_t, 0x00608128) = 0;
}
GP4_IMPL(0x00421ef8, FUN_00421ef8, "regs(eax:u32) -> (eax); globals=fuzz:i32")
