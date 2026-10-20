#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the pair of dwords at 0x0096548c/0x00965490 (or zeros when the byte flag [0x007ab754] is
// clear) into the four (x) and four (y) slots 0x009654b8/e0/cc/f4 and 0x009654bc/e4/d0/f8.
// eax is pushed/popped; edx is left holding the y value; the flags are those of
// `test byte [0x7ab754], 0xff` (mov/push/pop do not change them).
// FUNCTION: GP4 0x005516a6
void __cdecl FUN_005516a6(gp4::Regs* r) {
    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007ab754);
    uint32_t x = 0u;
    uint32_t y = 0u;
    if (enabled != 0u) {
        x = GP4_GLOBAL(uint32_t, 0x0096548c);
        y = GP4_GLOBAL(uint32_t, 0x00965490);
    }
    GP4_GLOBAL(uint32_t, 0x009654b8) = x;
    GP4_GLOBAL(uint32_t, 0x009654e0) = x;
    GP4_GLOBAL(uint32_t, 0x009654cc) = x;
    GP4_GLOBAL(uint32_t, 0x009654f4) = x;
    GP4_GLOBAL(uint32_t, 0x009654bc) = y;
    GP4_GLOBAL(uint32_t, 0x009654e4) = y;
    GP4_GLOBAL(uint32_t, 0x009654d0) = y;
    GP4_GLOBAL(uint32_t, 0x009654f8) = y;
    r->edx = y;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(enabled));
}
GP4_IMPL(0x005516a6, FUN_005516a6, "regs() -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
