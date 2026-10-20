#include <gp4/gp4.h>

// `call 0x0052a2c4 ; ret`: tail wrapper. The callee waits for the front-end screen (0x00527f80,
// 0x0053e4db, then the wait 0x0053c8af and a dispatch on its result); the poll reaches the message
// pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so PeekMessageA. Registers and flags come back
// as the callee leaves them.
// FUNCTION: GP4 0x0053371e
void __cdecl FUN_0053371e(gp4::Regs* r) {
    gp4::call_regs(0x0052a2c4, *r);
}
GP4_IMPL(0x0053371e, FUN_0053371e, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
