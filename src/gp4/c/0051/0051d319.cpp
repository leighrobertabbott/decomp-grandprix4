#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws the glyph for the character al through 0x0051d329 (text-cursor state machine and 7x8 glyph
// writer) only when the byte [0x7ad844] is 0xff; otherwise just the flags of
// `cmp byte [0x7ad844], 0xff` change. Registers and flags come back from the callee.
// FUNCTION: GP4 0x0051d319
void __cdecl FUN_0051d319(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ad844);
    if (mode != 0xffu) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0xffu));
        return;
    }
    gp4::call_regs(0x0051d329, *r);
}
GP4_IMPL(0x0051d319, FUN_0051d319, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
