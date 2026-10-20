#include <gp4/gp4.h>

// `call 0x00548a22 ; ret`: tail wrapper. The callee is a front-end screen loop (it rebuilds the list,
// waits through 0x0053c8af and dispatches on the key action); the wait reaches the message pump
// 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so PeekMessageA. Registers and flags come back as the
// callee leaves them.
// FUNCTION: GP4 0x00532a19
void __cdecl FUN_00532a19(gp4::Regs* r) {
    gp4::call_regs(0x00548a22, *r);
}
GP4_IMPL(0x00532a19, FUN_00532a19, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
