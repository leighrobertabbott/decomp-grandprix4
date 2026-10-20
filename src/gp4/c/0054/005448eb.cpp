#include <gp4/gp4.h>

// When [0x0083157c] is nonzero, returns the entry [0x007ab538] + 0xbc8 of the string-pointer table
// (looked up by the register routine 0x0054d1ad: esi = [0x835846 + 4 * esi]); otherwise 0. esi is
// saved and restored around the lookup.
// FUNCTION: GP4 0x005448eb
int __cdecl FUN_005448eb(void) {
    if (GP4_GLOBAL(uint32_t, 0x0083157c) == 0u) {
        return 0;
    }
    gp4::Regs c = {};
    c.esi = GP4_GLOBAL(uint32_t, 0x007ab538) + 0xbc8u;
    gp4::call_regs(0x0054d1ad, c);
    return static_cast<int>(c.esi);
}
GP4_IMPL(0x005448eb, FUN_005448eb, "cdecl() -> i32; globals=fuzz:i32+layout=lang_005448eb")
