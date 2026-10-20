#include <gp4/gp4.h>

// `mov eax, 0x2df ; call 0x00535c56 ; ret`: tail wrapper. eax = the screen code 0x2df goes to the
// screen setup 0x00535c56, which fills the screen record at 0x0081e504 and waits through 0x0053c8c8; that
// poll reaches the message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so PeekMessageA. Registers
// and flags come back as the callee leaves them.
// FUNCTION: GP4 0x00532a25
void __cdecl FUN_00532a25(gp4::Regs* r) {
    r->eax = 0x2dfu;
    gp4::call_regs(0x00535c56, *r);
}
GP4_IMPL(0x00532a25, FUN_00532a25, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
