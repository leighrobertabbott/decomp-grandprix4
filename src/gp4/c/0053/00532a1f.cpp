#include <gp4/gp4.h>

// `call 0x00531b41 ; ret`: tail wrapper. The callee waits through 0x0053c8af / 0x0053c8c8, which poll
// 0x0053d350; that poll reaches the message pump 0x00509b50 (via 0x0054a2c3) and so PeekMessageA.
// Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x00532a1f
void __cdecl FUN_00532a1f(gp4::Regs* r) {
    gp4::call_regs(0x00531b41, *r);
}
GP4_IMPL(0x00532a1f, FUN_00532a1f, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
