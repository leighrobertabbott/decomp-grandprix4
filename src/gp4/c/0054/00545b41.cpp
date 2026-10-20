#include <gp4/gp4.h>

// `movzx eax, word [0x7c2e96] ; call 0x00545b6a ; ret`: builds and draws the status line for the
// word [0x7c2e96] (the number in eax, formatted by 0x0054642c) from the text scripts 0xc1..0xc4 in
// the buffer at 0x823100 (0x00545b6a). Registers and flags come back from the callee.
// FUNCTION: GP4 0x00545b41
void __cdecl FUN_00545b41(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = GP4_GLOBAL(uint16_t, 0x007c2e96);
    gp4::call_regs(0x00545b6a, c);
    *r = c;
}
GP4_IMPL(0x00545b41, FUN_00545b41, "regs(ebx:u32, ecx:u32, edx:u32, esi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+surf_0054654a; emu=skip:win32 TextOutA")
