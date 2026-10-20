#include <gp4/gp4.h>
#include <gp4/flags.h>

// Picks the 0x007c8db0 table pointer by the renderer byte [0x007ad869]: 0x007f9b50 when it is zero,
// 0x007ff728 otherwise. eax is saved and restored; the flags are those of `cmp byte [0x7ad869], 0`.
// FUNCTION: GP4 0x0051cb4c
void __cdecl FUN_0051cb4c(gp4::Regs* r) {
    const uint8_t renderer = GP4_GLOBAL(uint8_t, 0x007ad869);
    uint32_t table;
    if (renderer == 0u) {
        table = 0x007f9b50u;
    } else {
        table = 0x007ff728u;
    }
    GP4_GLOBAL(uint32_t, 0x007c8db0) = table;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(renderer, 0u));
}
GP4_IMPL(0x0051cb4c, FUN_0051cb4c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
