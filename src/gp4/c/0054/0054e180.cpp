#include <gp4/gp4.h>

// Initialises the 800x600 display-mode record (width 800 at 0x822244, height 600 at 0x822248,
// 0x640 at 0x82224c, pixel counts 480000/960000 at 0x822254/0x822250), then calls the three
// register routines 0x54cb71 (eax = 1; no effect), 0x54cb4a (al = 0) and 0x54cc96 (eax = 0x7fff)
// and clears the byte [0x833cd6]. eax = 0x7fff is left behind and the flags are those the last
// routine (0x54cc96, `add edx, edx`) leaves; the final byte store does not touch them.
// FUNCTION: GP4 0x0054e180
void __cdecl FUN_0054e180(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00822244) = 0x320u;
    GP4_GLOBAL(uint32_t, 0x0082224c) = 0x640u;
    GP4_GLOBAL(uint32_t, 0x00822248) = 0x258u;
    GP4_GLOBAL(uint32_t, 0x00822254) = 0x75300u;
    GP4_GLOBAL(uint32_t, 0x00822250) = 0xea600u;

    gp4::Regs c = *r;
    c.eax = 1u;
    gp4::call_regs(0x0054cb71, c);
    c.eax = 0u;
    gp4::call_regs(0x0054cb4a, c);
    c.eax = 0x7fffu;
    gp4::call_regs(0x0054cc96, c);
    GP4_GLOBAL(uint8_t, 0x00833cd6) = 0u;
    *r = c;
}
GP4_IMPL(0x0054e180, FUN_0054e180, "regs(ebx:u32, ecx:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=col_0054cc37")
