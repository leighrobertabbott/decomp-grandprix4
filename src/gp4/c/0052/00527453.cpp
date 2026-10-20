#include <gp4/gp4.h>
#include <gp4/flags.h>

// Start-up: pushal; fninit (x87 control word 0x037f); fnstcw to 0x006f29a6; the control word with bit
// 11 cleared and bit 10 set (round down) is stored at 0x006f29a4 (0x077f). popal restores every
// register; the flags are those of the final `or eax, 0x400` (result 0x077f).
// FUNCTION: GP4 0x00527453
void __cdecl FUN_00527453(gp4::Regs* r) {
    const uint32_t cw = 0x037fu;
    GP4_GLOBAL(uint16_t, 0x006f29a6) = static_cast<uint16_t>(cw);
    const uint32_t rounded_down = (cw & 0xf7ffu) | 0x400u;
    GP4_GLOBAL(uint16_t, 0x006f29a4) = static_cast<uint16_t>(rounded_down);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(rounded_down));
}
GP4_IMPL(0x00527453, FUN_00527453, "regs() -> (cf, pf, af, zf, sf, of)")
