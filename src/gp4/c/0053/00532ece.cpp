#include <gp4/gp4.h>

// `call 0x005327c9 ; ret`: tail wrapper. The callee asks 0x0054acd3 and, when it returns CF, runs
// 0x00527f80 / 0x005327ec / the wait 0x0053c8af / 0x0054ad3c; the wait reaches the message pump
// 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so PeekMessageA. Registers and flags come back as the
// callee leaves them.
// FUNCTION: GP4 0x00532ece
void __cdecl FUN_00532ece(gp4::Regs* r) {
    gp4::call_regs(0x005327c9, *r);
}
GP4_IMPL(0x00532ece, FUN_00532ece, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
