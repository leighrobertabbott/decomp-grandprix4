#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the 0x954164.. render-mode bytes. With [0x7ad81c] == 0xff or the hardware-renderer byte
// [0x7ad869] nonzero, [0x954164] = 2 and [0x954165] = 0; otherwise 1 and 0xff. Then [0x95416d] =
// [0x954167] = 0, [0x95416a] takes [0x954164] when it was nonzero and [0x95416f] mirrors
// [0x7ad869] (0xff / 0). pushal/popal keeps every register; the flags are those of the last
// `cmp byte [0x7ad869], 0`.
// FUNCTION: GP4 0x0051ae07
void __cdecl FUN_0051ae07(gp4::Regs* r) {
    const uint8_t hw = GP4_GLOBAL(uint8_t, 0x007ad869);
    if (GP4_GLOBAL(uint8_t, 0x007ad81c) == 0xffu || hw != 0u) {
        GP4_GLOBAL(uint8_t, 0x00954164) = 2u;
        GP4_GLOBAL(uint8_t, 0x00954165) = 0u;
    } else {
        GP4_GLOBAL(uint8_t, 0x00954164) = 1u;
        GP4_GLOBAL(uint8_t, 0x00954165) = 0xffu;
    }
    GP4_GLOBAL(uint8_t, 0x0095416d) = 0u;
    GP4_GLOBAL(uint8_t, 0x00954167) = 0u;
    if (GP4_GLOBAL(uint8_t, 0x0095416a) != 0u) {
        GP4_GLOBAL(uint8_t, 0x0095416a) = GP4_GLOBAL(uint8_t, 0x00954164);
    }
    const uint8_t hw2 = GP4_GLOBAL(uint8_t, 0x007ad869);
    GP4_GLOBAL(uint8_t, 0x0095416f) = hw2 != 0u ? 0xffu : 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(hw2, 0u));
}
GP4_IMPL(0x0051ae07, FUN_0051ae07, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
