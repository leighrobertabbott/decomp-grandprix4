#include <gp4/gp4.h>

// `mov eax, 0x305 ; call 0x00535c56 ; ret`: tail wrapper. The callee builds the screen request with
// eax = 0x305 and waits through 0x0053c8c8, which polls 0x0053d350; that poll reaches the message pump
// 0x00509b50 (via 0x0054a2c3) and so PeekMessageA. Registers and flags come back as the callee leaves
// them.
// FUNCTION: GP4 0x00532a30
void __cdecl FUN_00532a30(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0x305u;
    gp4::call_regs(0x00535c56, c);
    *r = c;
}
GP4_IMPL(0x00532a30, FUN_00532a30, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
