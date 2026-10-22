#include <gp4/gp4.h>

// Stores (dword at 0x0066a668) + 0x16 into the dword global at 0x00abc0f8 (value left in eax).
// FUNCTION: GP4 0x005019a0
uint32_t __cdecl FUN_005019a0() {
    uint32_t v = GP4_GLOBAL(uint32_t, 0x0066a668) + 0x16;
    GP4_GLOBAL(uint32_t, 0x00abc0f8) = v;
    return v;
}
GP4_IMPL(0x005019a0, FUN_005019a0, "cdecl() -> u32; globals=fuzz:i32")
