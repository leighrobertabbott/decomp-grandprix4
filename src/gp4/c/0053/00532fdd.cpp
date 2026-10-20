#include <gp4/gp4.h>

// `call 0x00534ff0 ; ret`: tail wrapper. The callee is a front-end screen event loop (it polls
// 0x0053c8af and dispatches through a jump table); the poll reaches the message pump 0x00509b50
// (via 0x0053d350 / 0x0054a2c3) and so PeekMessageA. Registers and flags come back as the callee
// leaves them.
// FUNCTION: GP4 0x00532fdd
void __cdecl FUN_00532fdd(gp4::Regs* r) {
    gp4::call_regs(0x00534ff0, *r);
}
GP4_IMPL(0x00532fdd, FUN_00532fdd, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
