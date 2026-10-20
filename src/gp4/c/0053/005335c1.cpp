#include <gp4/gp4.h>

// `call 0x00548cd7 ; ret`: tail wrapper. The callee is `call 0x00527f80 ; call 0x0053c8af ; ret`: a
// single wait for the front-end screen; the poll reaches the message pump 0x00509b50 (via
// 0x0053d350 / 0x0054a2c3) and so PeekMessageA. Registers and flags come back as the callee leaves
// them.
// FUNCTION: GP4 0x005335c1
void __cdecl FUN_005335c1(gp4::Regs* r) {
    gp4::call_regs(0x00548cd7, *r);
}
GP4_IMPL(0x005335c1, FUN_005335c1, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
