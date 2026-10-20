#include <gp4/gp4.h>

// Looks up entry [0x007ab4b0] + 0xbc4 of the dword table that 0x0054d1ad indexes with esi, and returns
// it. push/pop keeps esi; the only caller (0x00507000) overwrites the flags the lookup leaves.
// FUNCTION: GP4 0x0054e56b
uint32_t __cdecl FUN_0054e56b() {
    gp4::Regs c = {};
    c.esi = GP4_GLOBAL(uint32_t, 0x007ab4b0) + 0xbc4u;
    gp4::call_regs(0x0054d1ad, c);
    return c.esi;
}
GP4_IMPL(0x0054e56b, FUN_0054e56b, "cdecl() -> u32; globals=layout=idx_007ab4b0")
