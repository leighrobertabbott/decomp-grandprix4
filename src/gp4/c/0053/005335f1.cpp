#include <gp4/gp4.h>

// `call 0x00540e8d ; ret`: tail wrapper. The callee is a front-end screen loop (it waits on 0x0053c8af,
// sets [0x00830029] = 1 and dispatches through a jump table); the poll reaches the message pump
// 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so PeekMessageA. Registers and flags come back as the
// callee leaves them.
// FUNCTION: GP4 0x005335f1
void __cdecl FUN_005335f1(gp4::Regs* r) {
    gp4::call_regs(0x00540e8d, *r);
}
GP4_IMPL(0x005335f1, FUN_005335f1, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
