#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws the text line of a front-end screen: runs 0x00545e54 with edi = the buffer 0x00823100 (it
// builds the line there, and terminates it through edi), then the text drawer 0x005461b9 with
// esi = the same buffer. Registers and flags carry from the first call into the second and come back
// from the second.
// FUNCTION: GP4 0x00545e3f
void __cdecl FUN_00545e3f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = 0x00823100u;
    gp4::call_regs(0x00545e54, c);
    c.esi = 0x00823100u;
    gp4::call_regs(0x005461b9, c);
    *r = c;
}
GP4_IMPL(0x00545e3f, FUN_00545e3f, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+cnt_00548836+script_0054d798+surf_0054654a; emu=skip:win32 TextOutA")
