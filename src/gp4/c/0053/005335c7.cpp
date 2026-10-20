#include <gp4/gp4.h>

// `call 0x0053fa27 ; ret`: tail wrapper. The callee sets [0x007ab020] = 1 and runs a front-end screen
// loop (0x00541c1f, then 0x00540131 / 0x00540269 and a wait on 0x0053c8af dispatching through a jump
// table); the poll reaches the message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so
// PeekMessageA. Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x005335c7
void __cdecl FUN_005335c7(gp4::Regs* r) {
    gp4::call_regs(0x0053fa27, *r);
}
GP4_IMPL(0x005335c7, FUN_005335c7, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
